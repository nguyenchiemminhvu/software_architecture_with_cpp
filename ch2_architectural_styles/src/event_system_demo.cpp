// event_system_demo.cpp
//
// Demonstrates a simplified event-driven architecture:
//   - Typed event bus (broker topology)
//   - Publisher/subscriber decoupling
//   - Event sourcing for an order aggregate
//
// Book reference: Chapter 2, §2.4 (Exploring event-based architecture)

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

// ============================================================
// EVENT BASE + TYPED EVENT BUS
// ============================================================

struct event_base {
    virtual ~event_base() = default;
};

// Lightweight in-process event bus (broker topology)
class event_bus {
    using handler_list = std::vector<std::function<void(const event_base&)>>;
    std::unordered_map<std::type_index, handler_list> handlers_;
public:
    template<typename EventT, typename HandlerFn>
    void subscribe(HandlerFn&& fn) {
        handlers_[std::type_index(typeid(EventT))].emplace_back(
            [fn = std::forward<HandlerFn>(fn)](const event_base& e) {
                fn(static_cast<const EventT&>(e));
            }
        );
    }

    template<typename EventT>
    void publish(const EventT& event) {
        auto it = handlers_.find(std::type_index(typeid(EventT)));
        if (it != handlers_.end()) {
            for (const auto& handler : it->second) {
                handler(event);
            }
        }
    }
};

// ============================================================
// DOMAIN EVENTS
// ============================================================

struct order_placed : event_base {
    int         order_id;
    std::string customer;
    double      amount;
    order_placed(int id, std::string c, double a)
        : order_id(id), customer(std::move(c)), amount(a) {}
};

struct payment_processed : event_base {
    int         order_id;
    std::string transaction_id;
    payment_processed(int id, std::string tx)
        : order_id(id), transaction_id(std::move(tx)) {}
};

struct order_shipped : event_base {
    int         order_id;
    std::string tracking_number;
    order_shipped(int id, std::string trk)
        : order_id(id), tracking_number(std::move(trk)) {}
};

// ============================================================
// SERVICES (subscribers + publishers)
// ============================================================

class inventory_service {
    event_bus& bus_;
public:
    explicit inventory_service(event_bus& bus) : bus_(bus) {
        bus_.subscribe<order_placed>([this](const order_placed& e) {
            on_order_placed(e);
        });
    }
private:
    void on_order_placed(const order_placed& e) {
        std::cout << "[Inventory] Reserving stock for order #" << e.order_id << "\n";
    }
};

class payment_service {
    event_bus& bus_;
public:
    explicit payment_service(event_bus& bus) : bus_(bus) {
        bus_.subscribe<order_placed>([this](const order_placed& e) {
            on_order_placed(e);
        });
    }
private:
    void on_order_placed(const order_placed& e) {
        std::cout << "[Payment] Processing payment of " << e.amount
                  << " for order #" << e.order_id << "\n";
        // Simulate successful payment
        payment_processed done{e.order_id, "TXN-" + std::to_string(e.order_id * 100)};
        bus_.publish(done);
    }
};

class shipping_service {
    event_bus& bus_;
public:
    explicit shipping_service(event_bus& bus) : bus_(bus) {
        bus_.subscribe<payment_processed>([this](const payment_processed& e) {
            on_payment_processed(e);
        });
    }
private:
    void on_payment_processed(const payment_processed& e) {
        std::cout << "[Shipping] Scheduling shipment for order #" << e.order_id
                  << " (txn: " << e.transaction_id << ")\n";
        order_shipped shipped{e.order_id, "TRACK-" + std::to_string(e.order_id)};
        bus_.publish(shipped);
    }
};

class notification_service {
    event_bus& bus_;
public:
    explicit notification_service(event_bus& bus) : bus_(bus) {
        bus_.subscribe<order_shipped>([this](const order_shipped& e) {
            on_order_shipped(e);
        });
    }
private:
    void on_order_shipped(const order_shipped& e) {
        std::cout << "[Notification] Email: order #" << e.order_id
                  << " shipped! Tracking: " << e.tracking_number << "\n";
    }
};

// ============================================================
// EVENT SOURCING — Order aggregate with event replay
// ============================================================
struct item_added : event_base {
    std::string product;
    double price;
    item_added(std::string p, double pr) : product(std::move(p)), price(pr) {}
};

struct order_confirmed : event_base {};

class order_aggregate {
    std::vector<std::unique_ptr<event_base>> event_log_;
    std::vector<std::string> items_;
    double total_ = 0.0;
    bool confirmed_ = false;

    void apply(const item_added& e) {
        items_.push_back(e.product);
        total_ += e.price;
    }
    void apply(const order_confirmed&) { confirmed_ = true; }

public:
    void add_item(const std::string& product, double price) {
        event_log_.push_back(std::make_unique<item_added>(item_added{product, price}));
        apply(static_cast<const item_added&>(*event_log_.back()));
    }
    void confirm() {
        event_log_.push_back(std::make_unique<order_confirmed>());
        apply(static_cast<const order_confirmed&>(*event_log_.back()));
    }
    void print_state() const {
        std::cout << "\n[EventSourced Order] Items:\n";
        for (const auto& item : items_) std::cout << "  - " << item << "\n";
        std::cout << "  Total: " << total_
                  << " | Confirmed: " << (confirmed_ ? "yes" : "no") << "\n";
    }
    int event_count() const { return static_cast<int>(event_log_.size()); }
};

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== Event-Driven Architecture Demo ===\n\n";

    // --- Broker event bus ---
    std::cout << "--- Event Bus: OrderPlaced cascade ---\n";
    event_bus bus;

    inventory_service    inventory(bus);
    payment_service      payment(bus);
    shipping_service     shipping(bus);
    notification_service notifier(bus);

    // Placing an order triggers the full cascade automatically
    bus.publish(order_placed{101, "Alice", 149.99});

    // --- Event Sourcing ---
    std::cout << "\n--- Event Sourcing: Order Aggregate ---\n";
    order_aggregate order;
    order.add_item("Widget Pro", 59.99);
    order.add_item("Gadget X",   89.99);
    order.confirm();
    order.print_state();
    std::cout << "  Events in log: " << order.event_count()
              << " (can replay to reconstruct any past state)\n";

    std::cout << "\nDone.\n";
    return 0;
}
