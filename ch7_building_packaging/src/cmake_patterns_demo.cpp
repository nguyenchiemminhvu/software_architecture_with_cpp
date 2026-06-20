// cmake_patterns_demo.cpp
//
// Demonstrates build-system-aware patterns:
//   - Compile-time version information embedded via CMake
//   - Platform detection macros
//   - Debug vs Release build switching
//
// Book reference: Chapter 7, §7.3-7.4 (CMake, build patterns)

#include <iostream>
#include <string_view>

// ============================================================
// Version information — typically injected by CMake configure_file
// In a real project, version.h would be generated from CMakeLists.txt
// ============================================================

namespace version {
    constexpr int major_v = 1;
    constexpr int minor_v = 2;
    constexpr int patch_v = 3;
    constexpr std::string_view full = "1.2.3";
    constexpr std::string_view build_type =
#ifdef NDEBUG
        "Release";
#else
        "Debug";
#endif
}

// ============================================================
// Platform detection
// ============================================================

constexpr std::string_view platform_name() {
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#else
    return "Unknown";
#endif
}

// ============================================================
// Debug-only instrumentation (compiled away in Release)
// ============================================================

#ifdef NDEBUG
    #define DEBUG_LOG(msg) do {} while(false)
#else
    #define DEBUG_LOG(msg) std::cout << "[DEBUG] " << (msg) << "\n"
#endif

// ============================================================
// Demonstrating target properties propagation
// (In a real multi-library project, these would be separate targets)
// ============================================================

class math_library {
public:
    double compute(double a, double b) const {
        DEBUG_LOG("math_library::compute called");
        return a * b + a / (b != 0 ? b : 1);
    }
};

class application {
    math_library math_;
public:
    void run() {
        std::cout << "Version: " << version::full
                  << " [" << version::build_type << "]\n";
        std::cout << "Platform: " << platform_name() << "\n";
        std::cout << "Result: " << math_.compute(4.0, 3.0) << "\n";
        DEBUG_LOG("Application::run completed");
    }
};

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== CMake Patterns Demo ===\n\n";
    std::cout << "This demo shows how CMake properties flow through targets.\n";
    std::cout << "In a real project:\n";
    std::cout << "  - target_compile_features sets the C++ standard\n";
    std::cout << "  - target_include_directories propagates include paths\n";
    std::cout << "  - target_compile_options sets warning flags per-target\n\n";

    application app;
    app.run();

    std::cout << "\nBuild notes:\n";
    std::cout << "  cmake -DCMAKE_BUILD_TYPE=Release .. (adds -O2 -DNDEBUG)\n";
    std::cout << "  cmake -DCMAKE_BUILD_TYPE=Debug ..   (adds -g, DEBUG_LOG active)\n";
    std::cout << "  cmake -DCMAKE_CXX_CLANG_TIDY=clang-tidy .. (enables static analysis)\n";

    return 0;
}
