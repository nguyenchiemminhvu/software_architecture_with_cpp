cmake_minimum_required(VERSION 3.16)
project(ch5_cpp_language_features LANGUAGES CXX)

add_executable(ch5_raii_demo
    src/raii_demo.cpp
)
target_compile_features(ch5_raii_demo PRIVATE cxx_std_20)
target_include_directories(ch5_raii_demo PRIVATE src)

add_executable(ch5_optional_demo
    src/optional_demo.cpp
)
target_compile_features(ch5_optional_demo PRIVATE cxx_std_20)
target_include_directories(ch5_optional_demo PRIVATE src)

add_executable(ch5_type_safety_demo
    src/type_safety_demo.cpp
)
target_compile_features(ch5_type_safety_demo PRIVATE cxx_std_20)
target_include_directories(ch5_type_safety_demo PRIVATE src)
