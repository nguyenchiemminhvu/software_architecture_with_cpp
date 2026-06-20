# Software Architecture with C++
## Study Guide with Practical Code Examples

**Source**: *Software Architecture with C++* by Adrian Ostrowski & Piotr Gaczkowski (Packt, 2021)

---

## Overview

This study guide summarizes the key ideas, architecture concepts, design patterns, and practical
techniques from Ostrowski & Gaczkowski's *Software Architecture with C++*. Each chapter directory
contains:

- A `README.md` with a detailed summary, PlantUML architecture diagrams, anti-patterns, and trade-offs
- Compilable C++ source files demonstrating each concept (where applicable)
- A `CMakeLists.txt` to build the examples in that chapter

The book covers the full spectrum of modern C++ software architecture: from design principles and
architectural styles, through C++ language features and design patterns, to building, testing,
securing, and deploying cloud-native systems.

---

## Audience and Learning Goals

**Target audience**:
- C++ developers with 2+ years of experience wanting to level up to architectural thinking
- Architects and tech leads who want practical C++ grounding for design decisions
- Teams transitioning from monolithic to service-oriented or cloud-native architectures

**After completing this guide you will be able to**:
- Apply SOLID/DRY/DDD principles concretely in C++ systems
- Choose and justify architectural styles (monolith, microservices, event-driven, layered)
- Elicit, classify, and document functional and nonfunctional requirements
- Design resilient distributed systems using fault-tolerance patterns
- Leverage modern C++ (C++17/20) features to write safer, faster, more expressive code
- Implement classic design patterns idiomatically in C++
- Build, package, and ship C++ code with modern CMake and Conan
- Write highly testable C++ with TDD, mocks, and test doubles
- Secure C++ codebases against common vulnerabilities (OWASP Top 10)
- Profile and optimize C++ programs from micro to system level
- Design SOA, microservices, containerized, and cloud-native architectures

---

## How to Use This Guide

1. **Linear reading**: Follow chapter order (1 → 15). The book naturally progresses from concepts
   to language details to systems.
2. **Targeted reference**: Use the chapter `README.md` files as standalone references for specific
   topics.
3. **Hands-on code**: Build and run each chapter's demos. Read the source before the output, then
   trace the output back to the source.
4. **Workshop mode**: Use `workshop_slides.md` and `speaking_content.md` to run an internal team
   workshop. Use `index.html` for a visual slide presentation.

---

## Directory Structure

```
software_architecture_with_cpp/
├── README.md                                    ← This file
├── CMakeLists.txt                               ← Top-level CMake
├── workshop_slides.md                           ← Slide-deck content
├── speaking_content.md                          ← Speaker notes / script
├── index.html                                   ← Interactive slide show
│
├── ch1_architecture_principles/                 ← Ch 1: SOLID, DDD, coupling/cohesion
├── ch2_architectural_styles/                    ← Ch 2: Monolith, microservices, events
├── ch3_functional_nonfunctional_requirements/   ← Ch 3: Requirements, 4+1, C4
├── ch4_architectural_system_design/             ← Ch 4: Distributed, CQRS, caching
├── ch5_cpp_language_features/                   ← Ch 5: RAII, optional, ranges, types
├── ch6_design_patterns_cpp/                     ← Ch 6: Factory, observer, CRTP, type erasure
├── ch7_building_packaging/                      ← Ch 7: CMake, Conan, static analysis
├── ch8_writing_testable_code/                   ← Ch 8: TDD, mocks, test doubles
├── ch9_cicd/                                    ← Ch 9: CI/CD pipelines (conceptual)
├── ch10_security/                               ← Ch 10: OWASP, secure coding
├── ch11_performance/                            ← Ch 11: Benchmarks, profiling, cache
├── ch12_service_oriented_architecture/          ← Ch 12: SOA, REST, messaging
├── ch13_designing_microservices/                ← Ch 13: Decomposition, observability
├── ch14_containers/                             ← Ch 14: Docker, K8s (conceptual)
└── ch15_cloud_native_design/                    ← Ch 15: Cloud-native principles
```

---

## Core Concepts Summary

| Chapter | Key Concept | One-Line Summary |
|---------|-------------|------------------|
| 1 | Architecture Principles | SOLID, DRY, coupling/cohesion, DDD, C++ philosophy |
| 2 | Architectural Styles | Monolith, microservices, event-driven, layered, module-based |
| 3 | Requirements | Functional vs NFR, ASRs, 4+1 model, C4 model |
| 4 | System Design | Distributed fallacies, CAP, CQRS, caching, fault tolerance |
| 5 | C++ Language Features | RAII, optional, ranges, constexpr, type safety, modules |
| 6 | Design Patterns | Factory, observer, CRTP, type erasure, PIMPL |
| 7 | Build & Package | CMake, Conan, compilers, static analysis, sanitizers |
| 8 | Testable Code | TDD, testing pyramid, mocks, fakes, contract testing |
| 9 | CI/CD | Pipelines, gating, GitLab CI, Ansible, Packer, Terraform |
| 10 | Security | OWASP, secure design, memory safety, sandboxing, DevSecOps |
| 11 | Performance | Benchmarking, profiling, cache-friendly code, parallelism |
| 12 | SOA | SOA patterns, messaging, REST, MQTT, ZeroMQ, gRPC |
| 13 | Microservices | Decomposition, observability, logging, distributed tracing |
| 14 | Containers | Docker, Kubernetes, container orchestration |
| 15 | Cloud-Native | CNCF, service mesh, Kubernetes, cloud-native patterns |

---

## Building All Examples

```bash
# From the project root
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

**Prerequisites**:
- CMake ≥ 3.16
- A C++20-capable compiler (GCC ≥ 10, Clang ≥ 11, MSVC 2019+)
- For ch8: Google Test (optional — examples use a lightweight inline test harness)

**Run a specific chapter demo**:
```bash
./ch5_raii_demo
./ch6_factory_demo
./ch11_perf_demo
```

---

## Prerequisites

- **Language**: Solid C++14 knowledge; C++17/20 familiarity helpful
- **Tools**: `cmake`, `make` or `ninja`, a modern compiler
- **Concepts**: Familiarity with OOP, templates, STL
- **Optional for later chapters**: Docker, Kubernetes basics; CI/CD tool exposure

---

## C++ Standard Used

Examples use **C++20** where beneficial (concepts, ranges, coroutines) but fall back to C++17
idioms for broader compiler compatibility. The CMake standard is set to `cxx_std_20`.

---

## Key Design Philosophy

> *"Software architecture is about making fundamental structural choices that are costly to change
> once implemented. Good architecture enables the system to evolve without catastrophic rewrites."*
> — paraphrased from Ostrowski & Gaczkowski

The three pillars of this book's approach:
1. **Design principles first** — SOLID, DRY, and DDD give the vocabulary for every decision
2. **Choose the right style** — architectural style is a strategic, not tactical, decision
3. **Quality attributes are first-class** — testability, security, and performance are designed
   in, not bolted on
