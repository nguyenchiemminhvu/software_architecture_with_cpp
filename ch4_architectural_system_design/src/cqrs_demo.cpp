// cqrs_demo.cpp
//
// Demonstrates CQRS (Command-Query Responsibility Segregation):
//   - Commands mutate state (write model)
//   - Queries read from a separate, denormalized read model
//   - A simple circuit breaker
//
// Book reference: Chapter 4, §4.5 (CQRS and event sourcing)

#include <chrono>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// ============================================================
// DOMAIN MODEL (write side)
// ============================================================

struct order_item {
    std::string product;
    int         quantity;
    double      unit_price;
};

struct order {
    int                      id;
    std::string              customer;
    std::vector<order_item>  items;
    bool                     confirmed = false;

    double total() const {
        double sum = 0.0;
        for (const auto& item : items)
            sum += item.quantity * item.unit_price;
        return sum;
    }
};

// ============================================================
// COMMANDS
// ============================================================

struct place_order_command {
    int         order_id;
    std::string customer;
    std::vector<order_item> items;
};

struct confirm_order_command {
    int order_id;
};

// ============================================================
// EVENTS (published after successful command handling)
// ============================================================

struct order_placed_event {
    int         order_id;
    std::string customer;
    double      total;
};

struct order_confirmed_event {
    int order_id;
};

// ============================================================
// WRITE SIDE (command handler + write store)
// ============================================================

class order_write_store {
    std::unordered_map<int, order> orders_;
public:
    void save(const order& o) { orders_[o.id] = o; }
    std::optional<order> find(int id) const {
        auto it = orders_.find(id);
        if (it == orders_.end()) return std::nullopt;
        return it->second;
    }
};

class order_command_handler {
    order_write_store& store_;
    std::vector<order_placed_event>    placed_events_;
    std::vector<order_confirmed_event> confirmed_events_;
public:
    explicit order_command_handler(order_write_store& store) : store_(store) {}

    void handle(const place_order_command& cmd) {
        order o{cmd.order_id, cmd.customer, cmd.items, false};
        store_.save(o);
        placed_events_.push_back({cmd.order_id, cmd.customer, o.total()});
        std::cout << "[Write] Order #" << cmd.order_id
                  << " placed for " << cmd.customer
                  << ", total=" << o.total() << "\n";
    }

    void handle(const confirm_order_command& cmd) {
        auto opt = store_.find(cmd.order_id);
        if (!opt) throw std::runtime_error("Order not found");
        auto o = *opt;
        o.confirmed = true;
        store_.save(o);
        confirmed_events_.push_back({cmd.order_id});
        std::cout << "[Write] Order #" << cmd.order_id << " confirmed\n";
    }

    // Events to project into the read model
    const std::vector<order_placed_event>&    placed_events()    { return placed_events_; }
    const std::vector<order_confirmed_event>& confirmed_events() { return confirmed_events_; }
};

// ============================================================
// READ SIDE (denormalized view optimized for queries)
// ============================================================

struct order_summary {
    int         id;
    std::string customer;
    double      total;
    bool        confirmed;
    int         item_count;
};

class order_read_model {
    std::unordered_map<int, order_summary> summaries_;
public:
    void apply(const order_placed_event& e) {
        // Build denormalized view — no joins needed at query time
        summaries_[e.order_id] = {e.order_id, e.customer, e.total, false, 0};
    }
    void apply(const order_confirmed_event& e) {
        auto it = summaries_.find(e.order_id);
        if (it != summaries_.end()) it->second.confirmed = true;
    }
    std::optional<order_summary> query(int id) const {
        auto it = summaries_.find(id);
        if (it == summaries_.end()) return std::nullopt;
        return it->second;
    }
    std::vector<order_summary> query_all_confirmed() const {
        std::vector<order_summary> result;
        for (const auto& [id, summary] : summaries_)
            if (summary.confirmed) result.push_back(summary);
        return result;
    }
};

// ============================================================
// CIRCUIT BREAKER (simplified)
// ============================================================

enum class circuit_state { closed, open, half_open };

class circuit_breaker {
    circuit_state state_      = circuit_state::closed;
    int           failures_   = 0;
    int           threshold_  = 3;
    std::chrono::steady_clock::time_point open_since_;
    std::chrono::seconds timeout_{5};

public:
    bool allow_request() {
        if (state_ == circuit_state::open) {
            auto elapsed = std::chrono::steady_clock::now() - open_since_;
            if (elapsed > timeout_) {
                state_ = circuit_state::half_open;
                std::cout << "[CircuitBreaker] Transitioning to HALF-OPEN\n";
                return true;
            }
            std::cout << "[CircuitBreaker] OPEN — rejecting request\n";
            return false;
        }
        return true;
    }

    void on_success() {
        failures_ = 0;
        if (state_ == circuit_state::half_open) {
            state_ = circuit_state::closed;
            std::cout << "[CircuitBreaker] Probe succeeded → CLOSED\n";
        }
    }

    void on_failure() {
        ++failures_;
        std::cout << "[CircuitBreaker] Failure " << failures_
                  << "/" << threshold_ << "\n";
        if (failures_ >= threshold_) {
            state_ = circuit_state::open;
            open_since_ = std::chrono::steady_clock::now();
            std::cout << "[CircuitBreaker] Threshold reached → OPEN\n";
        }
    }

    std::string state_str() const {
        switch (state_) {
            case circuit_state::closed:    return "CLOSED";
            case circuit_state::open:      return "OPEN";
            case circuit_state::half_open: return "HALF-OPEN";
        }
        return "UNKNOWN";
    }
};

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== CQRS Demo ===\n\n";

    // --- CQRS ---
    std::cout << "--- Command Side (Write) ---\n";
    order_write_store     write_store;
    order_command_handler handler(write_store);

    handler.handle(place_order_command{
        1, "Alice",
        {{"Widget Pro", 2, 29.99}, {"Gadget X", 1, 89.99}}
    });
    handler.handle(place_order_command{
        2, "Bob",
        {{"Thingamajig", 3, 15.00}}
    });
    handler.handle(confirm_order_command{1});

    // Project events into read model
    order_read_model read_model;
    for (const auto& e : handler.placed_events())    read_model.apply(e);
    for (const auto& e : handler.confirmed_events()) read_model.apply(e);

    std::cout << "\n--- Query Side (Read) ---\n";
    auto summary = read_model.query(1);
    if (summary) {
        std::cout << "Order #" << summary->id
                  << " customer=" << summary->customer
                  << " total=" << summary->total
                  << " confirmed=" << (summary->confirmed ? "yes" : "no") << "\n";
    }

    auto confirmed = read_model.query_all_confirmed();
    std::cout << "Confirmed orders count: " << confirmed.size() << "\n";

    // --- Circuit Breaker ---
    std::cout << "\n--- Circuit Breaker ---\n";
    circuit_breaker cb;
    std::cout << "State: " << cb.state_str() << "\n";

    // Simulate failures
    for (int i = 0; i < 4; ++i) {
        if (cb.allow_request()) {
            cb.on_failure(); // simulate remote call failure
        }
    }
    std::cout << "State after failures: " << cb.state_str() << "\n";

    // Subsequent requests rejected without hitting the remote service
    cb.allow_request();

    std::cout << "\nDone.\n";
    return 0;
}
