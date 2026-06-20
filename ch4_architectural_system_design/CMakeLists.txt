cmake_minimum_required(VERSION 3.16)
project(ch4_architectural_system_design LANGUAGES CXX)

add_executable(ch4_cqrs_demo
    src/cqrs_demo.cpp
)
target_compile_features(ch4_cqrs_demo PRIVATE cxx_std_20)
target_include_directories(ch4_cqrs_demo PRIVATE src)
