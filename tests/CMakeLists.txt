cmake_minimum_required(VERSION 3.15)

# Find Google Test (optional)
find_package(GTest)

if(NOT GTest_FOUND)
    message(STATUS "Google Test not found. Skipping tests.")
    return()
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

# Test sources
set(TEST_SOURCES
    test_profiler.cpp
    test_analyzer.cpp
    test_workloads.cpp
)

# Create test executable
add_executable(pie_tests ${TEST_SOURCES})

target_link_libraries(pie_tests PRIVATE
    pie
    GTest::GTest
    GTest::Main
)

# Add tests
include(GoogleTest)
gtest_discover_tests(pie_tests)

# Add custom test target
add_custom_target(run_tests
    COMMAND pie_tests
    DEPENDS pie_tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running PIE tests"
)

