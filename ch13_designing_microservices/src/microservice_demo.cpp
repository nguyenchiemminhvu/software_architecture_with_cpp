// microservice_demo.cpp
//
// Demonstrates microservice design patterns in-process:
//   - Service decomposition: OrderService, PaymentService, InventoryService
//   - Async event bus for decoupled communication
//   - Saga pattern (orchestration style) with compensation
//   - Correlation ID / trace context propagation
//   - Structured log output (simulated JSON)
//
// Book reference: Chapter 13, §13.2-13.4

#include <functional>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

// ============================================================
// TRACE CONTEXT (correlation ID propagation)
// ============================================================

struct trace_context {
    std::string trace_id;
    int         span_depth = 0;
};

// ============================================================
// STRUCTURED LOGGER (simulated JSON output)
// ============================================================

void log(const std::string& service, const std::string& level,
         const std::string& msg, const trace_context& ctx) {
    std::cout << "{\"service\":\"" << service
              << "\",\"level\":\"" << level
              << "\",\"trace_id\":\"" << ctx.trace_id
              << "\",\"msg\":\"" << msg << "\"}\n";
}

// ============================================================
// DOMAIN EVENTS
// ============================================================

struct event_order_placed {
    int         order_id;
    std::string customer;
    double      amount;
    trace_context trace;
};

struct event_payment_charged {
    int         order_id;
    std::string transaction_id;
    trace_context trace;
};

struct event_payment_failed {
    int         order_id;
    std::string reason;
    trace_context trace;
};

struct event_stock_reserved {
    int order_id;
    trace_context trace;
};

struct event_order_confirmed {
    int order_id;
    trace_context trace;
};

struct event_order_cancelled {
    int order_id;
    std::string reason;
    trace_context trace;
};

// ============================================================
// ASYNC EVENT BUS (in-process broker)
// ============================================================

class event_bus {
    std::map<std::string, std::vector<std::function<void(const void*)>>> handlers_;
public:
    template<typename Event>
    void subscribe(std::function<void(const Event&)> handler) {
        std::string key = typeid(Event).name();
        handlers_[key].push_back([h = std::move(handler)](const void* raw) {
            h(*static_cast<const Event*>(raw));
        });
    }

    template<typename Event>
    void publish(const Event& event) {
        std::string key = typeid(Event).name();
        auto it = handlers_.find(key);
        if (it != handlers_.end())
            for (auto& h : it->second) h(&event);
    }
};

// ============================================================
// PAYMENT SERVICE
// ============================================================

class payment_service {
    event_bus& bus_;
    int next_tx_id_ = 1000;
public:
    explicit payment_service(event_bus& b) : bus_(b) {
        bus_.subscribe<event_order_placed>([this](const event_order_placed& e) {
            handle_order_placed(e);
        });
    }

    void handle_order_placed(const event_order_placed& e) {
        log("payment-service", "info",
            "Charging card for order " + std::to_string(e.order_id), e.trace);
        if (e.amount > 10000.0) {
            bus_.publish(event_payment_failed{e.order_id, "Limit exceeded", e.trace});
        } else {
            std::string tx = "TX-" + std::to_string(next_tx_id_++);
            bus_.publish(event_payment_charged{e.order_id, tx, e.trace});
        }
    }

    // Compensating transaction
    void refund(int order_id, const trace_context& ctx) {
        log("payment-service", "info",
            "Refunding order " + std::to_string(order_id), ctx);
    }
};

// ============================================================
// INVENTORY SERVICE
// ============================================================

class inventory_service {
    event_bus& bus_;
    std::unordered_map<int, bool> reserved_;
public:
    explicit inventory_service(event_bus& b) : bus_(b) {
        bus_.subscribe<event_payment_charged>([this](const event_payment_charged& e) {
            handle_payment_charged(e);
        });
    }

    void handle_payment_charged(const event_payment_charged& e) {
        log("inventory-service", "info",
            "Reserving stock for order " + std::to_string(e.order_id), e.trace);
        reserved_[e.order_id] = true;
        bus_.publish(event_stock_reserved{e.order_id, e.trace});
    }

    // Compensating transaction
    void release_stock(int order_id, const trace_context& ctx) {
        reserved_.erase(order_id);
        log("inventory-service", "info",
            "Stock released for order " + std::to_string(order_id), ctx);
    }
};

// ============================================================
// ORDER SERVICE (saga orchestrator)
// ============================================================

class order_service {
    event_bus&        bus_;
    payment_service&  payment_;
    inventory_service& inventory_;
    int next_id_ = 1;
public:
    order_service(event_bus& b, payment_service& p, inventory_service& inv)
        : bus_(b), payment_(p), inventory_(inv) {

        bus_.subscribe<event_payment_charged>([this](const event_payment_charged& e) {
            log("order-service", "info",
                "Payment confirmed for order " + std::to_string(e.order_id), e.trace);
        });
        bus_.subscribe<event_stock_reserved>([this](const event_stock_reserved& e) {
            log("order-service", "info",
                "Confirming order " + std::to_string(e.order_id), e.trace);
            bus_.publish(event_order_confirmed{e.order_id, e.trace});
        });
        bus_.subscribe<event_payment_failed>([this](const event_payment_failed& e) {
            log("order-service", "error",
                "Payment failed: " + e.reason, e.trace);
            bus_.publish(event_order_cancelled{e.order_id, e.reason, e.trace});
        });
    }

    void place_order(const std::string& customer, double amount) {
        trace_context ctx{"trace-" + std::to_string(next_id_)};
        int id = next_id_++;
        log("order-service", "info",
            "Placing order for " + customer + " amount=" + std::to_string(amount), ctx);
        bus_.publish(event_order_placed{id, customer, amount, ctx});
    }
};

// ============================================================
// NOTIFICATION SERVICE (independent consumer)
// ============================================================

class notification_service {
    event_bus& bus_;
public:
    explicit notification_service(event_bus& b) : bus_(b) {
        bus_.subscribe<event_order_confirmed>([this](const event_order_confirmed& e) {
            log("notification-service", "info",
                "Sending confirmation email for order " + std::to_string(e.order_id),
                e.trace);
        });
        bus_.subscribe<event_order_cancelled>([this](const event_order_cancelled& e) {
            log("notification-service", "info",
                "Sending cancellation email for order " + std::to_string(e.order_id),
                e.trace);
        });
    }
};

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== Microservice Demo ===\n\n";

    event_bus bus;
    payment_service     payments(bus);
    inventory_service   inventory(bus);
    order_service       orders(bus, payments, inventory);
    notification_service notifications(bus);

    std::cout << "--- Scenario 1: Successful order ---\n";
    orders.place_order("Alice", 199.99);

    std::cout << "\n--- Scenario 2: Large order (payment fails, saga compensates) ---\n";
    orders.place_order("Bob", 99999.99);

    std::cout << "\n--- Scenario 3: Another successful order ---\n";
    orders.place_order("Carol", 49.50);

    std::cout << "\nDone.\n";
    return 0;
}
