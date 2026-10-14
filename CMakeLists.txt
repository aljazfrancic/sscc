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

add_executable(sscc src/main.cpp)

# Strict warnings on every compiler. CI also passes
# -DCMAKE_COMPILE_WARNING_AS_ERROR=ON to turn them into errors.
if(MSVC)
    # /Zc:__cplusplus makes MSVC report the real standard in __cplusplus.
    target_compile_options(sscc PRIVATE /W4 /permissive- /Zc:__cplusplus)
else()
    target_compile_options(sscc PRIVATE -Wall -Wextra -Wpedantic)
endif()
