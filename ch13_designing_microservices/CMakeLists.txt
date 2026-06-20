cmake_minimum_required(VERSION 3.16)
project(ch13_designing_microservices LANGUAGES CXX)

add_executable(ch13_microservice_demo
    src/microservice_demo.cpp
)
target_compile_features(ch13_microservice_demo PRIVATE cxx_std_20)
target_include_directories(ch13_microservice_demo PRIVATE src/)
