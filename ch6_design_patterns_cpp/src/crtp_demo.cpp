// crtp_demo.cpp
//
// Demonstrates:
//   - CRTP (Curiously Recurring Template Pattern) for static polymorphism
//   - Policy-based design
//   - std::variant for type-safe sum types
//
// Book reference: Chapter 6, §6.3-6.4 (Policy-based design, CRTP)

#include <iostream>
#include <string>
#include <variant>
#include <vector>

// ============================================================
// 1. CRTP — Static Polymorphism (zero virtual dispatch cost)
// ============================================================

// Base: uses CRTP to call derived implementation without vtable
template<typename Derived>
class serialisable {
public:
    std::string serialise() const {
        return static_cast<const Derived*>(this)->to_json_impl();
    }
    void print() const {
        std::cout << serialise() << "\n";
    }
};

class user : public serialisable<user> {
    std::string name_;
    int age_;
public:
    user(std::string name, int age) : name_(std::move(name)), age_(age) {}
    std::string to_json_impl() const {
        return R"({"type":"user","name":")" + name_ +
               R"(","age":)" + std::to_string(age_) + "}";
    }
};

class product : public serialisable<product> {
    std::string sku_;
    double price_;
public:
    product(std::string sku, double price) : sku_(std::move(sku)), price_(price) {}
    std::string to_json_impl() const {
        return R"({"type":"product","sku":")" + sku_ +
               R"(","price":)" + std::to_string(price_) + "}";
    }
};

void demo_crtp() {
    std::cout << "--- CRTP: Static Polymorphism ---\n";
    user    u("Alice", 30);
    product p("WGT-001", 29.99);
    u.print();
    p.print();
    // No vtable. Calls resolved at compile time.
    std::cout << "No virtual dispatch — zero overhead\n";
}

// ============================================================
// 2. POLICY-BASED DESIGN
// ============================================================

// Policies: injectable at compile time, no runtime overhead
struct console_logger {
    void log(const std::string& msg) const {
        std::cout << "[Console] " << msg << "\n";
    }
};

struct null_logger {
    void log(const std::string&) const {} // silent in production
};

struct sync_lock {
    void lock()   { /* std::mutex::lock() in real code */ }
    void unlock() { /* std::mutex::unlock() in real code */ }
};

struct no_lock {
    void lock()   {}
    void unlock() {}
};

template<typename LogPolicy = console_logger,
         typename LockPolicy = no_lock>
class cache {
    LogPolicy  logger_;
    LockPolicy lock_;
    std::vector<std::pair<int, std::string>> data_;
public:
    void put(int key, const std::string& value) {
        lock_.lock();
        data_.emplace_back(key, value);
        logger_.log("put key=" + std::to_string(key));
        lock_.unlock();
    }
    std::string get(int key) const {
        for (const auto& [k, v] : data_)
            if (k == key) return v;
        return "";
    }
};

void demo_policy() {
    std::cout << "\n--- Policy-Based Design ---\n";
    // Development: with logging
    cache<console_logger, no_lock> dev_cache;
    dev_cache.put(1, "value_one");
    dev_cache.put(2, "value_two");

    // Production: silent (compiled away entirely by optimiser)
    cache<null_logger, no_lock> prod_cache;
    prod_cache.put(1, "value_one"); // no output
}

// ============================================================
// 3. std::variant — Type-Safe Sum Types
// ============================================================

struct success { std::string message; };
struct not_found_error { int id; };
struct network_error { int code; std::string reason; };

using operation_result = std::variant<success, not_found_error, network_error>;

operation_result fetch_user(int id) {
    if (id == 1) return success{"User Alice found"};
    if (id == 99) return network_error{503, "service unavailable"};
    return not_found_error{id};
}

void handle_result(const operation_result& result) {
    std::visit([](const auto& r) {
        using T = std::decay_t<decltype(r)>;
        if constexpr (std::is_same_v<T, success>) {
            std::cout << "  OK: " << r.message << "\n";
        } else if constexpr (std::is_same_v<T, not_found_error>) {
            std::cout << "  NOT FOUND: user#" << r.id << "\n";
        } else if constexpr (std::is_same_v<T, network_error>) {
            std::cout << "  NETWORK ERROR " << r.code << ": " << r.reason << "\n";
        }
    }, result);
}

void demo_variant() {
    std::cout << "\n--- std::variant: Type-Safe Results ---\n";
    handle_result(fetch_user(1));
    handle_result(fetch_user(42));
    handle_result(fetch_user(99));
    std::cout << "All cases handled exhaustively at compile time\n";
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== CRTP & Policy-Based Design Demo ===\n\n";
    demo_crtp();
    demo_policy();
    demo_variant();
    std::cout << "\nDone.\n";
    return 0;
}
