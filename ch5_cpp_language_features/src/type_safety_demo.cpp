// type_safety_demo.cpp
//
// Demonstrates strong types, enum class, and concepts to
// make interfaces safe and self-documenting.
//
// Book reference: Chapter 5, §5.6-5.7

#include <concepts>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

// ============================================================
// STRONG TYPES — prevent argument order errors
// ============================================================

// Tag types make each unit distinct at the type level
template<typename Tag, typename T = int>
class strong_type {
    T value_;
public:
    explicit strong_type(T v) : value_(v) {}
    T get() const { return value_; }
    bool operator==(const strong_type& o) const { return value_ == o.value_; }
    bool operator<(const strong_type& o)  const { return value_ < o.value_; }
};

struct width_tag  {};
struct height_tag {};
struct x_pos_tag  {};
struct y_pos_tag  {};

using width_t  = strong_type<width_tag>;
using height_t = strong_type<height_tag>;
using x_pos_t  = strong_type<x_pos_tag>;
using y_pos_t  = strong_type<y_pos_tag>;

// API is self-documenting and prevents swapped arguments
struct window {
    x_pos_t  x;
    y_pos_t  y;
    width_t  width;
    height_t height;
};

void create_window(x_pos_t x, y_pos_t y, width_t w, height_t h) {
    std::cout << "[window] pos=(" << x.get() << "," << y.get()
              << ") size=" << w.get() << "x" << h.get() << "\n";
}

void demo_strong_types() {
    std::cout << "--- Strong Types ---\n";
    create_window(x_pos_t{100}, y_pos_t{200}, width_t{1920}, height_t{1080});
    // The following would be a compile error — uncomment to verify:
    // create_window(width_t{1920}, height_t{1080}, x_pos_t{100}, y_pos_t{200});
    std::cout << "Type-safe: arguments cannot be silently swapped\n";
}

// ============================================================
// ENUM CLASS — scoped enumerations
// ============================================================

enum class log_level { trace, debug, info, warning, error, critical };
enum class render_mode { wireframe, solid, textured };

std::string_view to_string(log_level level) {
    switch (level) {
        case log_level::trace:    return "TRACE";
        case log_level::debug:    return "DEBUG";
        case log_level::info:     return "INFO";
        case log_level::warning:  return "WARNING";
        case log_level::error:    return "ERROR";
        case log_level::critical: return "CRITICAL";
    }
    return "UNKNOWN";
}

class logger {
    log_level min_level_;
public:
    explicit logger(log_level min_level) : min_level_(min_level) {}
    void log(log_level level, std::string_view message) {
        if (level >= min_level_) {
            std::cout << "[" << to_string(level) << "] " << message << "\n";
        }
    }
};

void demo_enum_class() {
    std::cout << "\n--- enum class Demo ---\n";
    logger l(log_level::info);
    l.log(log_level::debug,   "This is hidden (below min level)");
    l.log(log_level::info,    "Application started");
    l.log(log_level::warning, "Cache miss rate high");
    l.log(log_level::error,   "Database connection lost");
    // log_level values are scoped — cannot accidentally compare with int
}

// ============================================================
// CONCEPTS (C++20) — constrained templates
// ============================================================

// Concept: a type that can be serialised to a string
template<typename T>
concept serialisable = requires(const T& t) {
    { t.to_string() } -> std::convertible_to<std::string>;
};

// Concept: a numeric type
template<typename T>
concept numeric = std::is_arithmetic_v<T>;

// Constrained template — error message is clear when violated
template<serialisable T>
void print_entity(const T& entity) {
    std::cout << "Entity: " << entity.to_string() << "\n";
}

template<numeric T>
T clamp(T value, T lo, T hi) {
    return value < lo ? lo : value > hi ? hi : value;
}

struct product {
    std::string name;
    double price;
    std::string to_string() const {
        return name + " ($" + std::to_string(price) + ")";
    }
};

struct order {
    int id;
    std::string to_string() const {
        return "Order#" + std::to_string(id);
    }
};

void demo_concepts() {
    std::cout << "\n--- Concepts Demo ---\n";
    print_entity(product{"Widget Pro", 29.99});
    print_entity(order{42});

    std::cout << "clamp(15, 0, 10) = " << clamp(15, 0, 10) << "\n";
    std::cout << "clamp(3.5, 0.0, 5.0) = " << clamp(3.5, 0.0, 5.0) << "\n";

    // The following would NOT compile with a clear error message:
    // print_entity(42);  // int does not satisfy 'serialisable'
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== Type Safety Demo ===\n\n";
    demo_strong_types();
    demo_enum_class();
    demo_concepts();
    std::cout << "\nDone.\n";
    return 0;
}
