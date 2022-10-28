A header-only C++ container indexed by ranges. 

RangeMap
========

A 'RangeMap<K,V>' is a container/'data structure' where a value of type 'V' can be assigned to a range of contiguous keys of type 'K', 
in a memory and time efficient manner, by using binary trees.
So for example, in the case of 'RangeMap<int, char>' the range '10' to '20' can be assigned the value 'a' without having to do 10 actual
assignments. Furthermore, a maximum of two elements are added to the container for each new range assignment.
RangeMap also guarantees that stored consecutive map entries do not contain the same value, making it canonical.



Example
=======

```cpp

RangeMap<int,char> rangeMap { 'x' }      // 'x' is the default value that is assigned to all possible values of 'int'
std::cout << rangeMap[15] << std::endl;  // 'x' is printed


rangeMap.assign(10,20,'a');              // assign 'a' to the range [10,20[ (note: this includes 10 but excludes 20)


std::cout << rangeMap[15] << std::endl;  // for values 10 to 19, 'a' is printed
std::cout << rangeMap[9]  << std::endl;  // The default 'x' is printed since no range assignments have been done that included 9

```



Template Parameter Requirements
===============================

RangeMap has a minimum amount of requirements for the template parameters 'K' and 'V'.
Both parameters must be copyable and assignable. In addtion key type 'K' must be less-than comparable 
via 'operator<' and value type 'V' must be equality-comparable via 'operator=='.


Note that RangeMap uses [concepts](https://en.cppreference.com/w/cpp/language/constraints), therefore 
a compiler with support for C++20 is required.



Testing
=======

RangeMap includes multiple unit tests located in the **unit_tests** folder. They can be run like so:

```bash

git clone https://github.com/0x31313030/RangeMap
mkdir _build && cd _build
cmake ..
make
./unit_tests/AssignmentTests

```
