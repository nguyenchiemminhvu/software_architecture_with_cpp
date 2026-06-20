cmake_minimum_required(VERSION 3.16)
project(ch11_performance LANGUAGES CXX)

add_executable(ch11_perf_demo
    src/perf_demo.cpp
)
target_compile_features(ch11_perf_demo PRIVATE cxx_std_20)
target_include_directories(ch11_perf_demo PRIVATE src/)
target_compile_options(ch11_perf_demo PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -O2>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /O2>
)
