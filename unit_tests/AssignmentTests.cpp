#include <gtest/gtest.h>
#include "RangeMap/RangeMap.h"
#include <random>

#include <chrono>

bool checkContainerIsCanonical( const std::map<int, char>& map, const char defaultValue )
{
    if( 0 == map.size() ) { return true; }
    auto curIt        = map.cbegin();
    auto [key, value] = *curIt;

    if( value == defaultValue ) { return false; }

    auto prevIt = curIt;
    for( ++curIt; curIt != map.cend(); ++curIt )
    {
      const auto& [prevKey, prevValue] = *prevIt;
      const auto& [curKey,  curValue ] = *curIt;

      if(prevValue == curValue) { return false; }

      prevIt = curIt;
    }

  return true;
}


class RangeMapTestAssignment : public ::testing::Test 
{
 protected:

  std::string PrintResultAndExpectation(const std::array<char, 20> finalState)
  {
    std::stringstream out;

    out << "\nresult: [|";
    for( size_t i{0}; i<20; ++i )
    {
        out << rMap[i] << "|";
    }
    out << "]\n";

    out << "expect: [|";
    for( size_t i=0; i<20; ++i )
    {
        out << finalState[i] << "|";
    }
    out << "]\n";

    return out.str();
  };

  void TestRangeMapAssignment( const std::map<int,char>& initialState, const std::array<char, 20> finalState, const int keyBegin, const int keyEnd, const char value = 'a')
  {
    auto elem = initialState.begin();
    while( elem != initialState.end() )
    {
        auto cur_elem  = elem;
        auto next_elem = ++elem;
        if( next_elem != initialState.end() )
        {
            rMap.assign( cur_elem->first, next_elem->first, cur_elem->second );
        }
    }

    rMap.assign(keyBegin, keyEnd, value); 

    for( size_t idx{0}; idx < finalState.size(); ++idx )
    {        
        ASSERT_EQ( finalState[idx], rMap[idx] ) << "\nerror at idx " << idx << ": ('" << finalState[idx] << "' != '" << rMap[idx] << "' )\n" << PrintResultAndExpectation(finalState);
    }


    ASSERT_TRUE( checkContainerIsCanonical(rMap.data(), ' ' ) );

  };

  RangeMap<int, char> rMap {' '};

};



// Case 1:
// ----------------------------			
// [            b   c     s   ] <--- Two initial ranges with values 'b' and 'c' (where 's' is the default value, marking the end of the 'c' range).
//    |    |                    <--- New range begin and end locations, insert 'a'.
// [  a    s    b   c     s   ] <--- Ranges after insertion of 'a' at the above new range.
//
//
//          0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// result: [ | | | | |b|b|c|c| | | | | | | | | | | |]
// result: [ |a|a|a| |b|b|c|c| | | | | | | | | | | |]
TEST_F(RangeMapTestAssignment, RangeInsertBeginningNoOverlap)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ','a','a','a',' ','b','b','c','c',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {5,'b'}, {7,'c'}, {9,' '} };

  TestRangeMapAssignment( initialState, finalState, 1, 4);
}


// Case 2:
// ----------------------------			
// [  b    s         c     s   ]
//            |   |              <--- insert 'a'
// [  b    s  a   s  c     s   ]
//
//
//          0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// result: [ |b|b|b| | | | | | |c|c|c|c|c| | | | | |]
// result: [ |b|b|b| | |a|a| | |c|c|c|c|c| | | | | |]
TEST_F(RangeMapTestAssignment, RangeInsertMiddleNoOverlap)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ','b','b','b',' ',' ','a','a',' ',' ','c','c','c','c','c',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {1,'b'}, {4,' '}, {10,'c'}, {15,' '} };

  TestRangeMapAssignment( initialState, finalState, 6, 8);
}

// Case 3:
// ----------------------------
// [   b   c     s            ]
//                    |    |    <--- insert 'a'
// [   b   c     s    a    s  ]
//
//
//          0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// result: [ | |b|b|b|b|c|c|c| | | | | | | | | | | |]
// result: [ | |b|b|b|b|c|c|c| | | |a|a|a|a|a| | | |]
TEST_F(RangeMapTestAssignment, RangeInsertEndNoOverlap)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ','b','b','b','b','c','c','c',' ',' ',' ','a','a','a','a','a',' ',' ',' ' };
  std::map<int,char>         initialState { {2,'b'}, {6,'c'}, {9,' '} };

  TestRangeMapAssignment( initialState, finalState, 12, 17);
}


// Case 4:
// ----------------------------			
// [       b        c     s   ]
//    |    |                    <--- insert 'a'
// [  a    b        c     s   ]
//
//
//         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
//result: [ | | | | | | |b|b|b|b|b|c|c|c|c|c| | | |]
//result: [a|a|a|a|a|a|a|b|b|b|b|b|c|c|c|c|c| | | |]
TEST_F(RangeMapTestAssignment, RangeInsertBeginningNoOverlapButBoundary)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { 'a','a','a','a','a','a','a','b','b','b','b','b','c','c','c','c','c',' ',' ',' ' };
  std::map<int,char>         initialState { {7,'b'}, {12,'c'}, {17,' '} };

  TestRangeMapAssignment( initialState, finalState, 0, 7);
}

// Case 5:
// ----------------------------			
// [  b    s        c     s   ]
//         |        |           <--- insert 'a'
// [  b    a        c     s   ]
//
//
//         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
//result: [ | | |b|b|b|b|b| | | | | | | |c|c|c|c| |]
//result: [ | | |b|b|b|b|b|a|a|a|a|a|a|a|c|c|c|c| |]
TEST_F(RangeMapTestAssignment, RangeInsertMiddleNoOverlapButBoundary)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','b','b','b','b','b','a','a','a','a','a','a','a','c','c','c','c',' ' };
  std::map<int,char>         initialState { {3,'b'}, {8,' '}, {15,'c'}, {19,' '}};

  TestRangeMapAssignment( initialState, finalState, 8, 15);
}

// Case 6:
// ----------------------------			
// [    b        c     s      ]
//                     |  |     <--- insert 'a'
// [    b        c     a  s   ]
//
//
//         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
//result: [ | | | | | | |b|b|b|b|b|c|c|c|c|c| | | |]
//result: [ | | | | | | |b|b|b|b|b|c|c|c|c|c|a|a| |]
TEST_F(RangeMapTestAssignment, RangeInsertEndNoOverlapButBoundary)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ',' ',' ',' ',' ','b','b','b','b','b','c','c','c','c','c','a','a',' ' };
  std::map<int,char>         initialState { {7,'b'}, {12,'c'}, {17,' '} };

  TestRangeMapAssignment( initialState, finalState, 17, 19);
}


// Case 7:
// ----------------------------
// [  b              s        ]
//         |     |              <--- insert 'a'
// [  b    a     b   s        ]
//
//
//          0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// result: [ | | |b|b|b|b|b|b|b|b|b|b|b|b| | | | | |]
// result: [ | | |b|b|b|b|a|a|a|a|a|b|b|b| | | | | |]
TEST_F(RangeMapTestAssignment, RangeInsertMiddleOfExistingRange)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','b','b','b','b','a','a','a','a','a','b','b','b',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {3,'b'}, {15,' '} };

  TestRangeMapAssignment( initialState, finalState, 7, 12 );
}


// Case 8:
// ----------------------------			
// [  b    cd     e     s   ]
//       |     |               <--- insert 'a'
// [  b  a     ad e     s   ]
//
//
//          0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// result: [ | | |b|b|b|b|b|c|c|c|c|d|d|d|d|e|e| | |]
// result: [ | | |b|b|a|a|a|a|a|a|a|a|a|d|d|e|e| | |]
TEST_F(RangeMapTestAssignment, RangeInsertMiddleOfTwoExistingRanges)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','b','b','a','a','a','a','a','a','a','a','a','d','d','e','e',' ',' ' };
  std::map<int,char>         initialState { {3,'b'}, {8,'c'}, {12,'d'}, {16,'e'}, {18,' '} };

  TestRangeMapAssignment( initialState, finalState, 5, 14 );
}


// Case 9:
// ----------------------------			
// [  b    cd     e     s    ]
//       |                |    <--- insert 'a'
// [  b  a                s  ]
//
//
//          0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// result: [ | | |b|b|b|b|b|c|c|c|c|d|d|d|d|e|e| | |]
// result: [ | | |b|b|a|a|a|a|a|a|a|a|a|a|a|a|a|a| |]
TEST_F(RangeMapTestAssignment, RangeInsertStartMiddleOfExistingRangesAndEndOverwritesPreviousRanges)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','b','b','a','a','a','a','a','a','a','a','a','a','a','a','a','a',' ' };
  std::map<int,char>         initialState { {3,'b'}, {8,'c'}, {12,'d'}, {16,'e'}, {18,' '} };

  TestRangeMapAssignment( initialState, finalState, 5, 19 );
}



// Case 10:
// ----------------------------
// [  b    cd     e     s   ]
//       |        |           <--- insert 'a'
// [  b  a        e     s   ]
//
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |b|b|b|b|b|c|c|c|c|d|d|d|d|e|e| | |]
// insertion: [| | | |b|b|a|a|a|a|a|a|a|a|a|a|a|e|e| | |]
TEST_F(RangeMapTestAssignment, RangeInsertStartMiddleOfExistingRangeAndEndOnBeginningOfAnotherRange)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','b','b','a','a','a','a','a','a','a','a','a','a','a','e','e',' ',' ' };
  std::map<int,char>         initialState { {3,'b'}, {8,'c'}, {12,'d'}, {16,'e'}, {18,' '} };

  TestRangeMapAssignment( initialState, finalState, 5, 16 );
}


// Case 11:
// ----------------------------
// [  a                 s   ]
//       |        |           <--- insert 'a'
// [  a                 s   ]
//
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
// insertion: [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
TEST_F(RangeMapTestAssignment, RangeInsertMiddleOfExistingRangeWithSameValue)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',' ',' ' };
  std::map<int,char>         initialState { {3,'a'}, {18,' '} };

  TestRangeMapAssignment( initialState, finalState, 5, 10 );
}



// Case 12:
// ----------------------------			
// [  a                 s   ]
//    |                 |     <--- insert 'a'
// [  a                 s   ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
// insertion: [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
TEST_F(RangeMapTestAssignment, RangeInsertCoincidesExactlyWithPreviousRangeOfSameValue)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',' ',' ' };
  std::map<int,char>         initialState { {3,'a'}, {18,' '} };

  TestRangeMapAssignment( initialState, finalState, 3, 18 );
}


// Case 13:
// ----------------------------			
// [  b                 s   ]
//    |                 |     <--- insert 'a'
// [  a                 s   ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |b|b|b|b|b|b|b|b|b|b|b|b|b|b|b| | |]
// insertion: [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
TEST_F(RangeMapTestAssignment, RangeInsertCoincidesExactlyWithPreviousRangeOfDifferentValue)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',' ',' ' };
  std::map<int,char>         initialState { {3,'b'}, {18,' '} };

  TestRangeMapAssignment( initialState, finalState, 3, 18 );
}


// Case 14:
// ----------------------------			
// [  a                 s   ]
//    |          |             <--- insert 'a'
// [  a                 s   ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
// insertion: [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
TEST_F(RangeMapTestAssignment, RangeInsertWhereBeginingOfRangeCoincidesWithPreviousRangeOfSameValue)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',' ',' ' };
  std::map<int,char>         initialState { {3,'a'}, {18,' '} };

  TestRangeMapAssignment( initialState, finalState, 3, 10 );
}


// Case 15:
// ----------------------------			
// [  a                 s   ]
//           |          |     <--- insert 'a'
// [  a                 s   ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
// insertion: [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
TEST_F(RangeMapTestAssignment, RangeInsertWhereEndOfRangeCoincidesWithPreviousRangeOfSameValue)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',' ',' ' };
  std::map<int,char>         initialState { {3,'a'}, {18,' '} };

  TestRangeMapAssignment( initialState, finalState, 10, 18 );
}


// Case 16:
// ----------------------------			
// [        a           s   ]
//    |          |             <--- insert 'a'
// [  a                 s   ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
// insertion: [| |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
TEST_F(RangeMapTestAssignment, RangeInsertWhereBeginOfRangeExpandsPreviousRangeOfSameValueToTheLeft)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',' ',' ' };
  std::map<int,char>         initialState { {3,'a'}, {18,' '} };

  TestRangeMapAssignment( initialState, finalState, 1, 10 );
}



// Case 17:
// ----------------------------			
// [    a         s         ]
//                |     |      <--- insert 'a'
// [    a               s   ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |a|a|a|a|a|a|a| | | | | | | | | | |]
// insertion: [| | | |a|a|a|a|a|a|a|a|a|a|a|a| | | | | |]
TEST_F(RangeMapTestAssignment, RangeInsertWhereRangeExpandsPreviousRangeWhereItEnded)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','a','a','a','a','a','a','a','a','a','a','a','a',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {3,'a'}, {10,' '} };

  TestRangeMapAssignment( initialState, finalState, 10, 15 );
}


// Case 18:
// ----------------------------			
// [    a         s         ]
//                 |     |    <--- insert 'a'
// [    a                s  ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |a|a|a|a|a|a|a| | | | | | | | | | |]
// insertion: [| | | |a|a|a|a|a|a|a| |a|a|a|a| | | | | |]
TEST_F(RangeMapTestAssignment, RangeInsertAfterLastExistingRange)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','a','a','a','a','a','a','a',' ','a','a','a','a',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {3,'a'}, {10,' '} };

  TestRangeMapAssignment( initialState, finalState, 11, 15 );
}

// Case 19:
// ----------------------------			
// [    a         s         ]
//               |     |      <--- insert 'a'
// [    a              s    ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |a|a|a|a|a|a|a| | | | | | | | | | |]
// insertion: [| | | |a|a|a|a|a|a|a|a|a|a|a|a| | | | | |]
TEST_F(RangeMapTestAssignment, RangeInsertExpandsPreviousRangeAtBoundary)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','a','a','a','a','a','a','a','a','a','a','a','a',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {3,'a'}, {10,' '} };

  TestRangeMapAssignment( initialState, finalState, 9, 15 );
}


// Case 20:
// ----------------------------
// [       a         s      ]
//    |    |                  <--- insert 'a'
// [  a              s      ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | | |a|a|a|a|a|a| | | | | | | | | | |]
// insertion: [| | |a|a|a|a|a|a|a|a| | | | | | | | | | |]
TEST_F(RangeMapTestAssignment, RangeInsertExpandsPreviousRangeWhereItStarted)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ','a','a','a','a','a','a','a','a',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {4,'a'}, {10,' '} };

  TestRangeMapAssignment( initialState, finalState, 2, 4 );
}

// Case 21:
// ----------------------------
// [       a         s      ]
//    |   |                   <--- insert 'a'
// [  a              s      ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | | |a|a|a|a|a|a| | | | | | | | | | |]
// insertion: [| |a|a| |a|a|a|a|a|a| | | | | | | | | | |]
TEST_F(RangeMapTestAssignment, RangeInsertNextToPreviousRangeWithOnlyOneGap)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ','a','a',' ','a','a','a','a','a','a',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {4,'a'}, {10,' '} };

  TestRangeMapAssignment( initialState, finalState, 1, 3 );
}

// Case 22:
// ----------------------------
// [       a         s      ]
//    |     |                 <--- insert 'a'
// [  a              s      ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | | |a|a|a|a|a|a| | | | | | | | | | |]
// insertion: [| |a|a|a|a|a|a|a|a|a| | | | | | | | | | |]
TEST_F(RangeMapTestAssignment, RangeInsertOverlapsPreviousRangeByOnlyOne)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ','a','a','a','a','a','a','a','a','a',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {4,'a'}, {10,' '} };

  TestRangeMapAssignment( initialState, finalState, 1, 5 );
}


// Case 23:
// ----------------------------
// [  a    s       a   s  ]
//           |   |          <--- insert 'a'
// [  a    s a   s a   s  ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | |a|a|a|a|a| | | | | | |a|a|a|a| | | |]
// insertion: [| | |a|a|a|a|a| |a|a|a|a| |a|a|a|a| | | |]
TEST_F(RangeMapTestAssignment, RangeInsertBetweenToPreviousRangesWithSameValueNoOverlap)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ','a','a','a','a','a',' ','a','a','a','a',' ','a','a','a','a',' ',' ',' ' };
  std::map<int,char>         initialState { {2,'a'}, {7,' '}, {13, 'a'}, {17,' '} };

  TestRangeMapAssignment( initialState, finalState, 8, 12 );
}


// Case 24:
// ----------------------------
// [  a    s       a   s  ]
//         |       |        <--- insert 'a'
// [  a                s  ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | |a|a|a|a|a| | | | | | |a|a|a|a| | | |]
// insertion: [| | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | | |]
TEST_F(RangeMapTestAssignment, RangeInsertBetweenToPreviousRangesWithSameValueAndOverlap)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',' ',' ',' ' };
  std::map<int,char>         initialState { {2,'a'}, {7,' '}, {13, 'a'}, {17,' '} };

  TestRangeMapAssignment( initialState, finalState, 6, 13 );
}


// Case 25:
// ----------------------------
// [ u  a  o  f  a  x  b s]
//         |         |      <--- insert 'a'
// [ u  a            a b s]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| |u|u|a|a|a|o|o|o|f|f|f|a|a|a|x|x|b|b| |]
// insertion: [| |u|u|a|a|a|a|a|a|a|a|a|a|a|a|a|x|b|b| |]
TEST_F(RangeMapTestAssignment, RangeInsertExpandsPreviousRangeWithSameValueOverMultipleRanges)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ','u','u','a','a','a','a','a','a','a','a','a','a','a','a','a','x','b','b',' ' };
  std::map<int,char>         initialState { {1,'u'}, {3,'a'}, {6,'o'}, {9,'f'}, {12,'a'}, {15,'x'}, {17,'b'}, {19, ' '} };

  TestRangeMapAssignment( initialState, finalState, 6, 16 );
}


// Case 26:
// ----------------------------
// [  a    s       a   s  ]
//         |      |         <--- insert 'a'
// [  a                s  ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | |a|a|a|a|a| | | | | | |a|a|a|a| | | |]
// insertion: [| | |a|a|a|a|a|a|a|a|a|a| |a|a|a|a| | | |]
TEST_F(RangeMapTestAssignment, RangeInsertBetweenToPreviousRangesWithSameValueAndPartialOverlap)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ','a','a','a','a','a','a','a','a','a','a',' ','a','a','a','a',' ',' ',' ' };
  std::map<int,char>         initialState { {2,'a'}, {7,' '}, {13, 'a'}, {17,' '} };

  TestRangeMapAssignment( initialState, finalState, 6, 12 );
}


// Case 27:
// ----------------------------			
// [           a      s   ]
//         |          |     <--- insert 'a'
// [       a          s   ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | | | | | | | | | |a|a|a|a| | | | | |]
// insertion: [| | | | | | | | | |a|a|a|a|a|a| | | | | |]
TEST_F(RangeMapTestAssignment, RangeInsertExtendsPreviousRangeToTheLeftWithOverlap)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ',' ',' ',' ',' ',' ',' ','a','a','a','a','a','a',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {11,'a'}, {15,' '} };

  TestRangeMapAssignment( initialState, finalState, 9, 15 );
}

// Case 28:
// ----------------------------
// [  a    s       a   s  ]
//           |   |          <--- insert 's' (default value)
// [  a    s       a   s  ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | |a|a|a|a|a| | | | | | |a|a|a|a| | | |]
// insertion: [| | |a|a|a|a|a| | | | | | |a|a|a|a| | | |]
TEST_F(RangeMapTestAssignment, RangeDeleteBetweenTwoNonContiguousPreviousRanges)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ','a','a','a','a','a',' ',' ',' ',' ',' ',' ','a','a','a','a',' ',' ',' ' };
  std::map<int,char>         initialState { {2,'a'}, {7,' '}, {13, 'a'}, {17,' '} };

  TestRangeMapAssignment( initialState, finalState, 8, 12, ' ' );
}


// Case 29:
// ----------------------------			
// [   a             s      ]
//         |     |            <--- insert 's' (default value)
// [   a   s     a   s      ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |a|a|a|a|a|a|a|a|a|a|a|a|a|a|a| | |]
// insertion: [| | | |a|a|a|a| | | | | | |a|a|a|a|a| | |]
TEST_F(RangeMapTestAssignment, RangeDeleteMiddleOfPreviousRange)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','a','a','a','a',' ',' ',' ',' ',' ',' ','a','a','a','a','a',' ',' ' };
  std::map<int,char>         initialState { {3,'a'}, {18,' '} };

  TestRangeMapAssignment( initialState, finalState, 7, 13, ' ' );
}

// Case 30:
// ----------------------------			
// [                        ]
//         |     |            <--- insert 's' (default value)
// [                        ]
//
// insertion range: [1,10[
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | | | | | | | | | | | | | | | | | | |]
// insertion: [| | | | | | | | | | | | | | | | | | | | |]
TEST_F(RangeMapTestAssignment, RangeDeleteEmptyContainer)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { };

  TestRangeMapAssignment( initialState, finalState, 7, 13, ' ' );
}


// Case 31:
// ----------------------------			
// [   a        s           ]
//           |          |       <--- insert 's' (default value)
// [   a     s              ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | |a|a|a|a|a|a|a|a|a|a|a|a| | | | | |]
// insertion: [| | | |a|a|a|a|a| | | | | | | | | | | | |]
TEST_F(RangeMapTestAssignment, RangeDeleteEndOfPreviousRange)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ','a','a','a','a','a',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {3,'a'}, {15,' '} };

  TestRangeMapAssignment( initialState, finalState, 8, 17, ' ' );
}


// Case 32:
// --------------------------
// [        a          s    ]
//       |       |            <--- insert 's' (default value)
// [             a     s    ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | | |a|a|a|a|a|a|a|a|a|a|a| | | | | |]
// insertion: [| | | | | | | | | |a|a|a|a|a|a| | | | | |]
TEST_F(RangeMapTestAssignment, RangeDeleteBeginningOfPreviousRange)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ',' ',' ',' ',' ',' ',' ','a','a','a','a','a','a',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {4,'a'}, {15,' '} };

  TestRangeMapAssignment( initialState, finalState, 2, 9, ' ' );
}

// Case 33:
// --------------------------
// [        a    b     s    ]
//          |      |          <--- insert 's' (default value)
// [               b   s    ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | | |a|a|a|a|b|b|b|b|b|b|b| | | | | |]
// insertion: [| | | | | | | | | | | |b|b|b|b| | | | | |]
TEST_F(RangeMapTestAssignment, RangeDeleteTwoContiguousPreviousRanges)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','b','b','b','b',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {4,'a'}, {7,'b'}, {15,' '} };

  TestRangeMapAssignment( initialState, finalState, 4, 11, ' ' );
}


// Case 34:
// --------------------------
// [ a       b       c    s ]
//        |      |            <--- insert 's' (default value)
// [ a    s      b   c    s ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| |a|a|a|a|a|a|b|b|b|b|b|b|b|c|c|c|c|c| |]
// insertion: [| |a|a|a|a| | | | | | |b|b|b|c|c|c|c|c| |]
TEST_F(RangeMapTestAssignment, RangeDeletePartsOfTwoContiguousPreviousRanges)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ','a','a','a','a',' ',' ',' ',' ',' ',' ','b','b','b','c','c','c','c','c',' ' };
  std::map<int,char>         initialState { {1,'a'}, {7,'b'}, {14,'c'}, {19, ' '} };

  TestRangeMapAssignment( initialState, finalState, 5, 11, ' ' );
}


// Case 35:
// --------------------------
// [      a         s       ]
//    |                  |    <--- insert 's' (default value)
// [                        ]
//
//              0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
// initial:   [| | | | |a|a|a|a|a|a|a|a|a| | | | | | | |]
// insertion: [| | | | | | | | | | | | | | | | | | | | |]
TEST_F(RangeMapTestAssignment, RangeDeleteOnlyRemainingPreviousRange)
{
  //                                         0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8   9
  const std::array<char, 20> finalState   { ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
  std::map<int,char>         initialState { {4,'a'}, {13, ' '} };

  TestRangeMapAssignment( initialState, finalState, 2, 15, ' ' );
}






void IntervalMapRandomTest()
{
  for(size_t it=0; it<5; ++it)
  {

    std::random_device rd;                            
    auto value { rd() };                                  // get a random number from device 
    //auto value { uint32_t(26131590) };
    std::mt19937 gen(value);                            // seed
    std::uniform_int_distribution<> distKey(-10000, 10000); // key range
    std::uniform_int_distribution<> distVal(0, 25);         // value range
    std::uniform_int_distribution<> distRsize(1, 100);      // range sizes
    printf("\nit=%lu | seed=%u\n--------------\n", it, value);

    const char defaultValue { 'g' };
    RangeMap<int,char> intmap { defaultValue };

    size_t total_time { 0 };
    size_t numIterations { 100'000 };
    for(size_t n=0; n<numIterations; ++n)
    {
        int rsize { distRsize(gen) };
        int pos   { distKey(gen)   };
        int c     { distVal(gen)   };
 
        auto t1 { std::chrono::high_resolution_clock::now() };
        intmap.assign( pos, pos+rsize, char('a'+c) );
        auto t2 { std::chrono::high_resolution_clock::now() };

        total_time += size_t(std::chrono::duration_cast<std::chrono::nanoseconds>(t2-t1).count());

        ASSERT_TRUE( checkContainerIsCanonical(intmap.data(), defaultValue ) ) << "\nFailed for random seed value: " << value << "\n";
    }

    std::cout <<  total_time/numIterations << std::endl;


  }
}



TEST(RandomRangeTests, test)
{
  IntervalMapRandomTest();
}


class DummyKey
{
public:
  DummyKey( const char var ) : mVar { var }
  {

  }

  DummyKey() = delete;
  
  friend bool operator<( DummyKey const& lhs, DummyKey const& rhs)
  {
        return lhs.mVar< rhs.mVar;
  }
  

private:
  char mVar;
};


class DummyValue
{
public:
  DummyValue( const int var ) : mVar { var }
  {

  }

  DummyValue(const DummyKey&) = delete;
  DummyValue() = delete;

  friend bool operator==( DummyValue const& lhs, DummyValue const& rhs)
  {
    return lhs.mVar == rhs.mVar;
  }


private:
  int mVar;
};


TEST(TemplateCompilationTest, CompilesWithKeyValueConstraints)
{
  RangeMap<DummyKey,DummyValue> intmap {' '};
  DummyKey   keyBegin {   5 };
  DummyKey   keyEnd   {  15 };
  DummyValue val      { 'f' };

  intmap.assign( keyBegin, keyEnd, val );

  ASSERT_TRUE( intmap.data().size() == 2 );
}

