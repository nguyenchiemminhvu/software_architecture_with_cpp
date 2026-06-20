cmake_minimum_required(VERSION 3.16)
project(ch6_design_patterns_cpp LANGUAGES CXX)

add_executable(ch6_factory_demo
    src/factory_demo.cpp
)
target_compile_features(ch6_factory_demo PRIVATE cxx_std_20)
target_include_directories(ch6_factory_demo PRIVATE src)

add_executable(ch6_observer_demo
    src/observer_demo.cpp
)
target_compile_features(ch6_observer_demo PRIVATE cxx_std_20)
target_include_directories(ch6_observer_demo PRIVATE src)

add_executable(ch6_crtp_demo
    src/crtp_demo.cpp
)
target_compile_features(ch6_crtp_demo PRIVATE cxx_std_20)
target_include_directories(ch6_crtp_demo PRIVATE src)
