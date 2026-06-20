// observer_demo.cpp
//
// Demonstrates the Observer pattern using:
//   1. std::function-based observers (modern C++ idiom)
//   2. Traditional interface-based observers
//   3. Copy-and-swap idiom for exception-safe assignment
//
// Book reference: Chapter 6, §6.5 (Tracking state and visiting objects)

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// ============================================================
// 1. MODERN OBSERVER — std::function callbacks
// ============================================================

enum class order_status { placed, payment_received, shipped, delivered, cancelled };

std::string to_string(order_status s) {
    switch (s) {
        case order_status::placed:            return "PLACED";
        case order_status::payment_received:  return "PAYMENT_RECEIVED";
        case order_status::shipped:           return "SHIPPED";
        case order_status::delivered:         return "DELIVERED";
        case order_status::cancelled:         return "CANCELLED";
    }
    return "UNKNOWN";
}

class order_tracker {
    int order_id_;
    order_status status_ = order_status::placed;
    std::vector<std::function<void(int, order_status)>> observers_;

public:
    explicit order_tracker(int id) : order_id_(id) {}

    // Subscribe — returns handle (index) for unsubscription
    size_t on_status_change(std::function<void(int, order_status)> handler) {
        observers_.push_back(std::move(handler));
        return observers_.size() - 1;
    }

    void set_status(order_status new_status) {
        status_ = new_status;
        for (const auto& obs : observers_) {
            obs(order_id_, status_);
        }
    }

    order_status status() const { return status_; }
    int id() const { return order_id_; }
};

void demo_functional_observer() {
    std::cout << "--- Functional Observer (std::function) ---\n";
    order_tracker order(42);

    // Lambda observer — logging
    order.on_status_change([](int id, order_status s) {
        std::cout << "  [Log] Order #" << id << " status: " << to_string(s) << "\n";
    });

    // Lambda observer — notification
    order.on_status_change([](int id, order_status s) {
        if (s == order_status::shipped) {
            std::cout << "  [Email] Order #" << id << " has shipped!\n";
        }
    });

    order.set_status(order_status::payment_received);
    order.set_status(order_status::shipped);
    order.set_status(order_status::delivered);
}

// ============================================================
// 2. INTERFACE-BASED OBSERVER (traditional GoF)
// ============================================================

class i_sensor_observer {
public:
    virtual void on_reading(float value) = 0;
    virtual ~i_sensor_observer() = default;
};

class temperature_sensor {
    float reading_ = 20.0f;
    std::vector<i_sensor_observer*> observers_; // non-owning
public:
    void subscribe(i_sensor_observer* obs) { observers_.push_back(obs); }
    void unsubscribe(i_sensor_observer* obs) {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), obs),
                         observers_.end());
    }
    void set_reading(float value) {
        reading_ = value;
        for (auto* obs : observers_) obs->on_reading(reading_);
    }
};

class alarm_observer : public i_sensor_observer {
    float threshold_;
public:
    explicit alarm_observer(float t) : threshold_(t) {}
    void on_reading(float value) override {
        if (value > threshold_)
            std::cout << "  [ALARM] Temperature " << value
                      << " exceeded threshold " << threshold_ << "\n";
    }
};

class logger_observer : public i_sensor_observer {
public:
    void on_reading(float value) override {
        std::cout << "  [LOG] Sensor reading: " << value << "°C\n";
    }
};

void demo_interface_observer() {
    std::cout << "\n--- Interface-Based Observer (GoF) ---\n";
    temperature_sensor sensor;
    logger_observer logger;
    alarm_observer  alarm(35.0f);

    sensor.subscribe(&logger);
    sensor.subscribe(&alarm);

    sensor.set_reading(22.0f);
    sensor.set_reading(38.5f);

    std::cout << "  Unsubscribing logger\n";
    sensor.unsubscribe(&logger);
    sensor.set_reading(40.0f);
}

// ============================================================
// 3. COPY-AND-SWAP IDIOM — exception-safe assignment
// ============================================================

class buffer {
    size_t size_;
    std::unique_ptr<char[]> data_;
public:
    explicit buffer(size_t size)
        : size_(size), data_(std::make_unique<char[]>(size)) {
        std::fill(data_.get(), data_.get() + size, 0);
    }
    buffer(const buffer& other)
        : size_(other.size_), data_(std::make_unique<char[]>(other.size_)) {
        std::copy(other.data_.get(), other.data_.get() + other.size_, data_.get());
    }
    buffer(buffer&&) noexcept = default;

    // Copy-and-swap: strongly exception-safe
    buffer& operator=(buffer other) {  // pass by value = copy or move
        swap(*this, other);
        return *this;
    }
    friend void swap(buffer& a, buffer& b) noexcept {
        using std::swap;
        swap(a.size_, b.size_);
        swap(a.data_, b.data_);
    }
    size_t size() const { return size_; }
};

void demo_copy_and_swap() {
    std::cout << "\n--- Copy-and-Swap Idiom ---\n";
    buffer b1(100);
    buffer b2(200);
    std::cout << "b1.size()=" << b1.size() << " b2.size()=" << b2.size() << "\n";
    b1 = b2;  // copy assignment via copy-and-swap
    std::cout << "After b1 = b2: b1.size()=" << b1.size() << " (strongly exception-safe)\n";
    buffer b3(50);
    b1 = std::move(b3);  // move assignment via copy-and-swap
    std::cout << "After b1 = move(b3): b1.size()=" << b1.size() << "\n";
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== Observer Pattern Demo ===\n\n";
    demo_functional_observer();
    demo_interface_observer();
    demo_copy_and_swap();
    std::cout << "\nDone.\n";
    return 0;
}
