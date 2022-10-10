cmake_minimum_required(VERSION 3.0.0)
project(RangeMap VERSION 0.1.0)

add_compile_options(-Wall -Wextra -pedantic -Werror -ansi)

set(CMAKE_CXX_STANDARD 20)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})



include(CTest)
include(CPack)

add_subdirectory(unit_tests)
