cmake_minimum_required(VERSION 3.16)
project(SoftwareArchitectureWithCpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# -------------------------------------------------------
# Section 1: Concepts and Components
# -------------------------------------------------------
add_subdirectory(ch1_architecture_principles)
add_subdirectory(ch2_architectural_styles)
# ch3 is conceptual — no compiled code

# -------------------------------------------------------
# Section 2: Design and Development of C++ Software
# -------------------------------------------------------
add_subdirectory(ch4_architectural_system_design)
add_subdirectory(ch5_cpp_language_features)
add_subdirectory(ch6_design_patterns_cpp)
add_subdirectory(ch7_building_packaging)

# -------------------------------------------------------
# Section 3: Architectural Quality Attributes
# -------------------------------------------------------
add_subdirectory(ch8_writing_testable_code)
# ch9 is CI/CD focused — no compiled C++ code
add_subdirectory(ch10_security)
add_subdirectory(ch11_performance)

# -------------------------------------------------------
# Section 4: Cloud-Native Design Principles
# -------------------------------------------------------
add_subdirectory(ch12_service_oriented_architecture)
add_subdirectory(ch13_designing_microservices)
# ch14 and ch15 are infrastructure-focused — no compiled C++ code

# -------------------------------------------------------
# Convenience target: build and run all demos
# -------------------------------------------------------
add_custom_target(run_all
    COMMAND ch1_solid_demo
    COMMAND ch2_event_system_demo
    COMMAND ch4_cqrs_demo
    COMMAND ch5_raii_demo
    COMMAND ch5_optional_demo
    COMMAND ch5_type_safety_demo
    COMMAND ch6_factory_demo
    COMMAND ch6_observer_demo
    COMMAND ch6_crtp_demo
    COMMAND ch7_cmake_patterns_demo
    COMMAND ch8_testing_demo
    COMMAND ch10_security_demo
    COMMAND ch11_perf_demo
    COMMAND ch12_soa_demo
    COMMAND ch13_microservice_demo
    DEPENDS
        ch1_solid_demo
        ch2_event_system_demo
        ch4_cqrs_demo
        ch5_raii_demo
        ch5_optional_demo
        ch5_type_safety_demo
        ch6_factory_demo
        ch6_observer_demo
        ch6_crtp_demo
        ch7_cmake_patterns_demo
        ch8_testing_demo
        ch10_security_demo
        ch11_perf_demo
        ch12_soa_demo
        ch13_microservice_demo
    COMMENT "Running all chapter demos"
)
