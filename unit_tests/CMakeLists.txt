include(GoogleTest)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#add_definitions(-D_GLIBCXX_DEBUG )
add_definitions(-DNDEBUG )

enable_testing()

add_executable(
  AssignmentTests
  AssignmentTests.cpp
)

target_link_libraries(
  AssignmentTests
  GTest::gtest_main
)

target_include_directories(AssignmentTests PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_compile_options(AssignmentTests PRIVATE -Wsign-conversion ) #-Wconversion )


gtest_discover_tests(AssignmentTests)
#add_test(ranged_map_gtests AssignmentTests)
