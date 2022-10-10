#pragma once

#include <map>
#include <type_traits>
#include <cassert>


template<typename T>
concept is_less_than_comparable =
    requires (T a, T b) 
    {
        { a < b } -> std::same_as<bool>;
    };

template<typename T>
concept is_equality_comparable =
    requires(T a, T b) 
    {
        { a == b } -> std::same_as<bool>;
    };


/**
 * @brief A container that associates ranges of value 'K' with values of 'V' in a memory and time 
 *        efficient manner.
 *        When looking up a value 'K', that falls inside a range, its value 'V' is returned, 
 *        otherwise a default value 'V' is returned (set in constructor).
 * 
 * @tparam K  The key type, must be copyable, assignable and less-than comparable via operator<
 * @tparam V  The value type, must be copyable, assignable and equality-comparable via operator==
 */
template<typename K, typename V>
    requires std::is_copy_assignable<K>::value &&
             std::is_copy_constructible<K>::value &&
                  is_less_than_comparable<K> &&

             std::is_copy_assignable<V>::value &&
             std::is_copy_constructible<V>::value &&
                  is_equality_comparable<V>
class RangeMap
{
  public:
    /**
     * @brief Construct a new Range Map object where the whole range of K
     *        is associated with value 'dafaultVal'.
     * 
     * @param dafaultVal The value which will be returned when looking up
     *                   K values that fall outside ranges.
     */
    RangeMap( V const& dafaultVal )
    : mDefaultVal { dafaultVal }
    {}



    /**
     * @brief Associate 'keyVal' to range ['keyBegin', 'keyEnd'[, overwriting 
     *        any previous values which overlap with this range. Ranges where 
     * 		  'keyBegin' < 'keyEnd' are ignored and does not change container. 
     * 		  The runtime for this call is amortized O(log N)
     * 
     * @param keyBegin  The start of the range.
     * @param keyEnd    The end of the range. Note that the range excludes 'keyEnd',
     *                  and therefore 'keyEnd' will not be assigned value 'keyVal'.
     * @param keyVal    The value to associate to the range ['keyBegin', 'keyEnd'[
     */
    void assign( K const& keyBegin, K const& keyEnd, V const& keyVal );



    /**
     * @brief Does a lookup of the value associated with 'key'
     * 
     * @param key  The key to lookup
     * @return     The value associated with 'key'
     */
    V const& operator[]( K const& key ) const;



    /**
     * @brief Return the underlying map container used to store the ranges. Modify 
     *        at own risk!
     */
    std::map<K,V>& data();



  private:
    /**
     * @brief Inserts the key that marks the beginning of a range and returns the iterator 
     * 		  position for the insertion. If the insertion position is at the start of 
     *        another range, then checks the value of the range before it, to see if it 
     *        should be extended, so as to avoid two consecutive map elements with the 
     *        same value, for example:
     *       
     *        [ 'a'    'b'         's'    ]  <--- two initial ranges with values 'a' and 'b' (where 's' is the default value, marking the end of the 'b' range)
     *                  |     |              <--- new range 'begin' and 'end' locations
     *        [ 'a'          'b'   's'    ]  <--- ranges after insertion of 'a' at the above new range
     *       
     *        ..where insertion is of value 'a', from start of range with value 'b', until middle
     *        of range. 
     * 
     * @param keyBegin     Where to insert the beginning of range
     * @param keyVal       The value to use for range
     * @param keyBeginPos  The upper bound position of 'keyBegin' in 'mMap' (hint)
     * @return             The iterator position for inserted 'keyBegin'
     */
    typename std::map<K, V>::iterator InsertKeyBegin( K const& keyBegin, V const& keyVal, typename std::map<K,V>::iterator const keyBeginPos );


    // Member variables
    const V       mDefaultVal; // Default value for values of 'K' that fall outside ranges
    std::map<K,V> mMap;        // Container used for storing the ranges
};




template<typename K, typename V>
void RangeMap<K,V>::assign( K const& keyBegin, K const& keyEnd, V const& keyVal )
{
    // ignore invalid range
    if( !(keyBegin < keyEnd) )
    {
        return;
    }

    // Find key positions and values in map:
    //
    //        keyBegin      keyEnd
    //            |           |
    //            ▼           ▼
    // [  'a'      's'    'b'      'c'  's'  ]  <-- std::map with current ranges
    //     |        |      |        |
    //     |        ▼      |        ▼
    //     |  keyBeginPos  |     keyEndPos
    //     ▼               ▼
    // keyBeginValue    KeyEndValue
    //
    auto keyBeginPos { mMap.upper_bound(keyBegin) }; // In which previous range is 'keyBegin' in?

    // Find in which previous range 'keyEnd' is in, by doing a linear search
    // from keyBeginPos. Benchmarks show that this is faster on average  
    // compared to doing: 'auto keyEndPos { mMap.upper_bound(keyEnd) };'
    auto keyEndPos = keyBeginPos; 
    while( keyEndPos != mMap.end() )
    {
        if( (keyEndPos->first < keyEnd) )
        {
            ++keyEndPos;
        }
        else if( keyEnd < keyEndPos->first )
        {
            break;
        }
        else // if( keyEnd == keyEndPos->first )
        {
            ++keyEndPos;
        }
    }


    const bool isKeyBeginWithinPreviousRange { (keyBeginPos != mMap.begin()) && (keyBeginPos != mMap.end()) };
    const bool isKeyEndWithinPreviousRanges  { (keyEndPos   != mMap.begin()) && (keyEndPos   != mMap.end()) };


    // check 'keyBegin' range value, before insertion of 'keyEnd' changes map
    bool beginRangeValueEqualKeyVal = false;
    if( isKeyBeginWithinPreviousRange )
    {
        const V& curRangeValue { std::prev(keyBeginPos)->second };
        beginRangeValueEqualKeyVal = (curRangeValue == keyVal);
    }


    // insert 'keyEnd'
    if( isKeyEndWithinPreviousRanges )
    {
        const V&   curRangeValue            = std::prev(keyEndPos)->second;
        const bool endRangeValueEqualKeyVal = (curRangeValue == keyVal);

        if( !endRangeValueEqualKeyVal )
        {
            keyEndPos = mMap.insert_or_assign( keyEndPos, keyEnd, curRangeValue );  // continue previous range, right after new range
        }
    }
    else
    {
        if( !(mDefaultVal == keyVal) )
        {
            keyEndPos = mMap.insert_or_assign( keyEndPos, keyEnd, mDefaultVal );
        }
    }

    // insert 'keyBegin'
    if( isKeyBeginWithinPreviousRange )
    {
        if( !beginRangeValueEqualKeyVal )
        {
            // update upper bound in case insertion of 'keyEnd' changed the bound for 'keyBegin'
            if( keyEndPos != mMap.end() && keyEndPos->first < keyBeginPos->first )
            {
                keyBeginPos = keyEndPos;
            }

            keyBeginPos = InsertKeyBegin( keyBegin, keyVal, keyBeginPos );
        }
        else
        {
            --keyBeginPos; // go to start of current range
        }
    }
    else
    {
        if( !(mDefaultVal == keyVal) )
        {
            // update upper bound in case insertion of 'keyEnd' changed the bound for 'keyBegin'
            if( keyBeginPos == mMap.end() || (keyEndPos != mMap.end() && keyEndPos->first < keyBeginPos->first) )
            {
                keyBeginPos = keyEndPos;
            }

            keyBeginPos = InsertKeyBegin( keyBegin, keyVal, keyBeginPos );
        }
    }


    // Delete any previous ranges between 'keyBeginPos' and 'keyEndPos'
    if( keyBeginPos != mMap.end() )
    {
        if( mDefaultVal == keyVal && keyBeginPos == mMap.begin() )
        {
            mMap.erase( keyBeginPos, keyEndPos );
        }
        else if( std::next(keyBeginPos) != keyEndPos )
        {
            mMap.erase( std::next(keyBeginPos), keyEndPos);
        }
    }
}



template<typename K, typename V>
V const& RangeMap<K,V>::operator[]( K const& key ) const
{
    auto it = mMap.upper_bound(key);

    if( it == mMap.begin() )
    {
        return mDefaultVal;
    }
    else
    {
        return (--it)->second;
    }
}



template<typename K, typename V>
std::map<K,V>& RangeMap<K,V>::data()
{
     return mMap;
}



template<typename K, typename V>
typename std::map<K, V>::iterator RangeMap<K,V>::InsertKeyBegin( K const& keyBegin, V const& keyVal, typename std::map<K,V>::iterator const keyBeginPos )
{
    typename std::map<K, V>::iterator out;

    auto insertionPos { mMap.insert( keyBeginPos, {keyBegin, keyVal} ) };
    
    assert( ((std::next(insertionPos) == mMap.end()) || (std::next(insertionPos) == keyBeginPos)) && ("std::map insert hint was not correct!") );

    if( insertionPos != mMap.begin() && std::prev(insertionPos)->second == keyVal )
    {
        out = std::prev(insertionPos); // previous range is being extended so no insertion of 'keyBegin'
    }
    else
    {
        // overwrite current range
        insertionPos->second = keyVal;
        out = insertionPos;
    }

    return out;
}
