# 3.16 matches the minimum of GoogleTest 1.18, which the unit tests will use.
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

# The checks live in a library without input or output, so the console
# program and the unit tests can share them.
add_library(sscc_core STATIC src/sscc.cpp src/report.cpp)
target_include_directories(sscc_core PUBLIC src)
target_link_libraries(sscc_core PRIVATE sscc_warnings)

add_executable(sscc src/main.cpp)
target_link_libraries(sscc PRIVATE sscc_core sscc_warnings)
