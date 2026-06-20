cmake_minimum_required(VERSION 3.16)
project(ch12_service_oriented_architecture LANGUAGES CXX)

add_executable(ch12_soa_demo
    src/soa_demo.cpp
)
target_compile_features(ch12_soa_demo PRIVATE cxx_std_20)
target_include_directories(ch12_soa_demo PRIVATE src/)
