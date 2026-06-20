cmake_minimum_required(VERSION 3.16)
project(ch1_architecture_principles LANGUAGES CXX)

add_executable(ch1_solid_demo
    src/solid_demo.cpp
)
target_compile_features(ch1_solid_demo PRIVATE cxx_std_20)
target_include_directories(ch1_solid_demo PRIVATE src)

add_executable(ch1_coupling_cohesion_demo
    src/coupling_cohesion_demo.cpp
)
target_compile_features(ch1_coupling_cohesion_demo PRIVATE cxx_std_20)
target_include_directories(ch1_coupling_cohesion_demo PRIVATE src)
