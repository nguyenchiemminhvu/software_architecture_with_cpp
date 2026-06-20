cmake_minimum_required(VERSION 3.16)
project(ch7_building_packaging LANGUAGES CXX)

# Demonstrates modern CMake patterns:
#   - target-based properties
#   - interface includes
#   - generator expressions for warning flags
#   - install rules

# --- Library target ---
add_library(ch7_example_lib STATIC
    src/cmake_patterns_demo.cpp
)
target_include_directories(ch7_example_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        src/
)
target_compile_features(ch7_example_lib PUBLIC cxx_std_20)
target_compile_options(ch7_example_lib PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# --- Executable that uses the library ---
add_executable(ch7_cmake_patterns_demo
    src/cmake_patterns_demo.cpp
)
target_compile_features(ch7_cmake_patterns_demo PRIVATE cxx_std_20)
target_include_directories(ch7_cmake_patterns_demo PRIVATE src/)
target_compile_options(ch7_cmake_patterns_demo PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
