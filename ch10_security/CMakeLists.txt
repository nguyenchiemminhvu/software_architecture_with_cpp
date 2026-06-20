cmake_minimum_required(VERSION 3.16)
project(ch10_security LANGUAGES CXX)

add_executable(ch10_security_demo
    src/security_demo.cpp
)
target_compile_features(ch10_security_demo PRIVATE cxx_std_20)
target_include_directories(ch10_security_demo PRIVATE src/)

# Enable security-hardening compile options
target_compile_options(ch10_security_demo PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:
        -Wall -Wextra -Wpedantic
        -Wformat-security
        -fstack-protector-strong
    >
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /GS>
)
