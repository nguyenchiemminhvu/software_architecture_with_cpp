// security_demo.cpp
//
// Demonstrates secure C++ coding patterns:
//   - Input validation at system boundaries
//   - Safe string/buffer handling with std::span
//   - Avoiding integer overflow
//   - Secure resource management (RAII)
//   - Safe concurrency with std::atomic
//
// Book reference: Chapter 10, §10.1-10.3

#include <atomic>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// ============================================================
// 1. INPUT VALIDATION AT SYSTEM BOUNDARIES
// ============================================================

struct validated_email {
    std::string value;
    static validated_email parse(const std::string& input) {
        // Minimal validation: contains @, non-empty local and domain parts
        auto at_pos = input.find('@');
        if (at_pos == std::string::npos || at_pos == 0 || at_pos == input.size() - 1)
            throw std::invalid_argument("invalid email: " + input);
        if (input.size() > 254)  // RFC 5321 limit
            throw std::invalid_argument("email exceeds maximum length");
        return validated_email{input};
    }
};

struct validated_port {
    uint16_t value;
    static validated_port parse(int raw) {
        if (raw < 1 || raw > 65535)
            throw std::out_of_range("port must be 1-65535, got " + std::to_string(raw));
        return validated_port{static_cast<uint16_t>(raw)};
    }
};

void demo_input_validation() {
    std::cout << "--- Input Validation ---\n";
    try {
        auto email = validated_email::parse("alice@example.com");
        std::cout << "  Valid email: " << email.value << "\n";
    } catch (const std::exception& e) {
        std::cout << "  Error: " << e.what() << "\n";
    }
    try {
        auto bad = validated_email::parse("not-an-email");
        (void)bad;
    } catch (const std::invalid_argument& e) {
        std::cout << "  Rejected: " << e.what() << "\n";
    }
    try {
        auto port = validated_port::parse(8080);
        std::cout << "  Valid port: " << port.value << "\n";
        auto bad_port = validated_port::parse(70000);
        (void)bad_port;
    } catch (const std::out_of_range& e) {
        std::cout << "  Rejected: " << e.what() << "\n";
    }
}

// ============================================================
// 2. SAFE BUFFER HANDLING WITH std::span
// ============================================================

// UNSAFE pattern (avoid): raw pointer + size — easy to mismatch
// void process_unsafe(const char* data, size_t size);

// SAFE: std::span enforces bounds; cannot exceed data.size()
size_t count_zeros(std::span<const uint8_t> data) {
    size_t count = 0;
    for (auto byte : data)
        if (byte == 0) ++count;
    return count;
}

void demo_span() {
    std::cout << "\n--- Safe Buffer Handling (std::span) ---\n";
    std::vector<uint8_t> buffer = {0x01, 0x00, 0xFF, 0x00, 0x42, 0x00};
    size_t zeros = count_zeros(buffer);
    std::cout << "  Zeros in buffer: " << zeros << " (safe: no raw pointer arithmetic)\n";

    // Subspan — also bounds-safe
    auto subspan = std::span<const uint8_t>(buffer).subspan(1, 3);
    std::cout << "  Zeros in subspan(1,3): " << count_zeros(subspan) << "\n";
}

// ============================================================
// 3. SAFE ARITHMETIC — checked integer operations
// ============================================================

template<typename T>
T safe_add(T a, T b) {
    if (b > 0 && a > std::numeric_limits<T>::max() - b)
        throw std::overflow_error("integer overflow in safe_add");
    if (b < 0 && a < std::numeric_limits<T>::min() - b)
        throw std::underflow_error("integer underflow in safe_add");
    return a + b;
}

template<typename T>
std::optional<T> safe_multiply(T a, T b) {
    if (a != 0 && b != 0) {
        if (a > std::numeric_limits<T>::max() / b) return std::nullopt;
    }
    return a * b;
}

void demo_safe_arithmetic() {
    std::cout << "\n--- Safe Integer Arithmetic ---\n";
    try {
        auto result = safe_add<int32_t>(2'000'000'000, 200'000'000);
        std::cout << "  safe_add(2B, 200M) = " << result << "\n";
        // This should overflow:
        auto overflow = safe_add<int32_t>(2'000'000'000, 2'000'000'000);
        (void)overflow;
    } catch (const std::overflow_error& e) {
        std::cout << "  Caught overflow: " << e.what() << "\n";
    }
    auto mul = safe_multiply<int32_t>(100'000, 100'000);
    if (!mul) std::cout << "  Multiplication overflowed — rejected\n";
    else      std::cout << "  100000 * 100000 = " << *mul << "\n";
}

// ============================================================
// 4. THREAD-SAFE COUNTER — atomic vs naive
// ============================================================

void demo_atomic() {
    std::cout << "\n--- Thread-Safe Atomic Counter ---\n";
    std::atomic<int> counter = 0;
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter] {
            for (int j = 0; j < 1000; ++j)
                ++counter; // atomic: no data race
        });
    }
    for (auto& t : threads) t.join();
    std::cout << "  Final counter (10 threads × 1000): " << counter
              << " (should be 10000)\n";
}

// ============================================================
// 5. SENSITIVE DATA — cleared from memory after use
// ============================================================

class secure_string {
    std::vector<char> data_;
public:
    explicit secure_string(std::string_view s) : data_(s.begin(), s.end()) {}
    std::string_view view() const { return {data_.data(), data_.size()}; }
    ~secure_string() {
        // Explicitly zero sensitive data before freeing
        std::fill(data_.begin(), data_.end(), '\0');
    }
    secure_string(const secure_string&) = delete;
    secure_string& operator=(const secure_string&) = delete;
};

void demo_secure_string() {
    std::cout << "\n--- Sensitive Data Handling ---\n";
    {
        secure_string password("super_secret_password_123");
        std::cout << "  Password in use: [redacted] (not printing)\n";
        // Do work with password here...
        // On scope exit, destructor zeros memory
    }
    std::cout << "  Password zeroed from memory on scope exit\n";
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== Security Demo ===\n\n";
    demo_input_validation();
    demo_span();
    demo_safe_arithmetic();
    demo_atomic();
    demo_secure_string();
    std::cout << "\nDone.\n";
    return 0;
}
