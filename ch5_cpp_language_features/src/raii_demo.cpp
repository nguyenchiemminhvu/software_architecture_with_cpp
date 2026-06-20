// raii_demo.cpp
//
// Demonstrates RAII for multiple resource types:
//   - File handle guard
//   - Lock guard (manual)
//   - Scope-exit guard (generalised cleanup)
//
// Book reference: Chapter 5, §5.2 (Leveraging RAII)

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

// ============================================================
// 1. FILE HANDLE RAII GUARD
// ============================================================

class file_guard {
    FILE* f_ = nullptr;
public:
    explicit file_guard(const char* path, const char* mode) {
        f_ = fopen(path, mode);
        if (!f_) throw std::runtime_error(std::string("cannot open: ") + path);
        std::cout << "[file_guard] opened " << path << "\n";
    }
    ~file_guard() {
        if (f_) {
            fclose(f_);
            std::cout << "[file_guard] closed\n";
        }
    }
    FILE* get() const { return f_; }
    // Non-copyable (owns exclusive resource)
    file_guard(const file_guard&) = delete;
    file_guard& operator=(const file_guard&) = delete;
    // Movable
    file_guard(file_guard&& other) noexcept : f_(other.f_) { other.f_ = nullptr; }
};

void demo_file_guard() {
    std::cout << "\n--- File Guard ---\n";
    // File automatically closed even if an exception is thrown
    try {
        file_guard f("/tmp/raii_test.txt", "w");
        fputs("hello raii\n", f.get());
        // f destroyed here — file closed
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
    }
    std::cout << "After scope — file is already closed\n";
}

// ============================================================
// 2. GENERIC SCOPE-EXIT GUARD
// ============================================================

class scope_exit {
    std::function<void()> cleanup_;
    bool active_ = true;
public:
    explicit scope_exit(std::function<void()> fn) : cleanup_(std::move(fn)) {}
    ~scope_exit() { if (active_) cleanup_(); }
    void dismiss() { active_ = false; } // cancel cleanup if commit succeeds
    // Non-copyable
    scope_exit(const scope_exit&) = delete;
    scope_exit& operator=(const scope_exit&) = delete;
};

void demo_scope_exit() {
    std::cout << "\n--- Scope Exit Guard ---\n";
    std::cout << "Entering scope\n";
    {
        scope_exit cleanup([] { std::cout << "[scope_exit] cleanup ran\n"; });
        std::cout << "Doing work inside scope\n";
        // cleanup runs automatically at scope exit
    }
    std::cout << "After scope\n";

    std::cout << "\n--- Scope Exit with dismiss (commit) ---\n";
    bool committed = false;
    {
        scope_exit rollback([&committed] {
            std::cout << "[scope_exit] rolling back transaction\n";
        });
        std::cout << "Performing transaction...\n";
        committed = true;
        if (committed) rollback.dismiss(); // success — no rollback needed
    }
    std::cout << "Transaction committed — no rollback\n";
}

// ============================================================
// 3. SHARED OWNERSHIP WITH std::shared_ptr
// ============================================================

struct heavy_resource {
    std::string name;
    heavy_resource(std::string n) : name(std::move(n)) {
        std::cout << "[heavy_resource] constructing " << name << "\n";
    }
    ~heavy_resource() {
        std::cout << "[heavy_resource] destroying " << name << "\n";
    }
};

void demo_shared_ptr() {
    std::cout << "\n--- Shared Ownership ---\n";
    std::shared_ptr<heavy_resource> owner1;
    {
        owner1 = std::make_shared<heavy_resource>("SharedWidget");
        std::cout << "ref count: " << owner1.use_count() << "\n"; // 1
        {
            auto owner2 = owner1; // shared ownership
            std::cout << "ref count: " << owner1.use_count() << "\n"; // 2
            // owner2 destroyed here — ref count drops to 1
        }
        std::cout << "ref count after inner scope: " << owner1.use_count() << "\n"; // 1
    }
    // owner1 destroyed here — resource destroyed
    std::cout << "After all owners released\n";
}

// ============================================================
// 4. MUTEX + RAII LOCK
// ============================================================

std::mutex g_mutex;
int g_counter = 0;

void demo_lock_guard() {
    std::cout << "\n--- Lock Guard ---\n";
    {
        std::lock_guard<std::mutex> lock(g_mutex);  // locked
        ++g_counter;
        std::cout << "Counter under lock: " << g_counter << "\n";
        // lock released at scope exit — even if exception thrown
    }
    std::cout << "Lock released\n";
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== RAII Demo ===\n";
    demo_file_guard();
    demo_scope_exit();
    demo_shared_ptr();
    demo_lock_guard();
    std::cout << "\nDone. All resources cleaned up automatically.\n";
    return 0;
}
