cmake_minimum_required(VERSION 3.16)
project(ch8_writing_testable_code LANGUAGES CXX)

# Uses a lightweight inline test harness (no GTest dependency required).
# In a real project, replace with GTest/Catch2 via FetchContent.
add_executable(ch8_testing_demo
    src/testing_demo.cpp
)
target_compile_features(ch8_testing_demo PRIVATE cxx_std_20)
target_include_directories(ch8_testing_demo PRIVATE src/)
