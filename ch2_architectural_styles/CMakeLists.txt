cmake_minimum_required(VERSION 3.16)
project(ch2_architectural_styles LANGUAGES CXX)

add_executable(ch2_event_system_demo
    src/event_system_demo.cpp
)
target_compile_features(ch2_event_system_demo PRIVATE cxx_std_20)
target_include_directories(ch2_event_system_demo PRIVATE src)
