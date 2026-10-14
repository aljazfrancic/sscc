# 3.16 matches the minimum of GoogleTest 1.18, which the unit tests use.
# The range also enables the newer CMake policies, up to CMake 4.2.
cmake_minimum_required(VERSION 3.16...4.2)

project(sscc
    VERSION 0.1.0
    DESCRIPTION "Checks scanned GS1-128 SSCC labels"
    LANGUAGES CXX)

# C++17 for every target, without compiler-specific extensions
# (-std=c++17 instead of -std=gnu++17), so the code stays portable.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict warnings for our own targets only, not for third-party code such as
# GoogleTest. CI also passes -DCMAKE_COMPILE_WARNING_AS_ERROR=ON.
add_library(sscc_warnings INTERFACE)
if(MSVC)
    target_compile_options(sscc_warnings INTERFACE /W4 /permissive-)
else()
    target_compile_options(sscc_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

# Everything except main() is in a library that the unit tests share with the
# console program. Only cli.cpp does input and output, through streams that
# the tests can replace.
add_library(sscc_core STATIC src/sscc.cpp src/report.cpp src/cli.cpp)
target_include_directories(sscc_core PUBLIC src)
target_link_libraries(sscc_core PRIVATE sscc_warnings)

add_executable(sscc src/main.cpp)
target_link_libraries(sscc PRIVATE sscc_core sscc_warnings)

# The unit tests use GoogleTest, which FetchContent downloads at a pinned
# version and checks against its hash. -DBUILD_TESTING=OFF builds without them,
# for example without an internet connection.
option(BUILD_TESTING "Build the unit tests" ON)
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/releases/download/v1.18.0/googletest-1.18.0.tar.gz
        URL_HASH SHA256=6e3191c1455468b3fc35a417fb565c1c5071aee1b7e7f85e30cf48a98d37d8b5)
    # On MSVC, GoogleTest must use the same C runtime library as our targets.
    # GoogleTest is only used here, so it is not installed with the project.
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(sscc_tests
        tests/gs1_test.cpp tests/sscc_test.cpp tests/report_test.cpp tests/cli_test.cpp)
    target_link_libraries(sscc_tests PRIVATE sscc_core sscc_warnings GTest::gmock_main)
    # The command line tests check the six task examples in the data file.
    target_compile_definitions(sscc_tests PRIVATE
        SSCC_EXAMPLES_FILE="${PROJECT_SOURCE_DIR}/data/examples.txt")

    # Each test becomes a separate CTest test, so ctest and the VS Code test
    # explorer show them one by one.
    enable_testing()
    include(GoogleTest)
    gtest_discover_tests(sscc_tests)

    # The unit tests call sscc::run(), so this test runs the real program once
    # to check main(). It passes if the output has the expected summary. The
    # exit status is ignored, because it is 1: four of the six labels are invalid.
    add_test(NAME Program.ChecksTheTaskExamples
        COMMAND sscc --file "${PROJECT_SOURCE_DIR}/data/examples.txt")
    set_tests_properties(Program.ChecksTheTaskExamples PROPERTIES
        PASS_REGULAR_EXPRESSION "Summary: 2 valid, 4 invalid\\.")
endif()
