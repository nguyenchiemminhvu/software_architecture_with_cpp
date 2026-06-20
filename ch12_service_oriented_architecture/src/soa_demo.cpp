// soa_demo.cpp
//
// Demonstrates service-oriented design patterns:
//   - Service interface + in-process stub (simulates remote call)
//   - Request/response model with error handling
//   - Simple pub/sub message bus (broker topology)
//   - Idempotent operation pattern
//
// Book reference: Chapter 12, §12.1-12.3

#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

// ============================================================
// SERVICE INTERFACE — defines the contract
// ============================================================

struct order_dto {
    int         id;
    std::string customer;
    double      amount;
    std::string status;
};

struct service_error {
    int         code;
    std::string message;
};

using order_result = std::variant<order_dto, service_error>;

class i_order_service {
public:
    virtual order_result get_order(int order_id) = 0;
    virtual order_result place_order(const std::string& customer, double amount) = 0;
    virtual ~i_order_service() = default;
};

// ============================================================
// IMPLEMENTATION — would be a real HTTP/gRPC call in production
// ============================================================

class local_order_service : public i_order_service {
    std::unordered_map<int, order_dto> orders_;
    int next_id_ = 1;
public:
    order_result get_order(int order_id) override {
        auto it = orders_.find(order_id);
        if (it == orders_.end())
            return service_error{404, "Order not found: " + std::to_string(order_id)};
        return it->second;
    }
    order_result place_order(const std::string& customer, double amount) override {
        if (amount <= 0) return service_error{400, "Amount must be positive"};
        int id = next_id_++;
        orders_[id] = {id, customer, amount, "placed"};
        return orders_[id];
    }
};

// ============================================================
// CLIENT — uses the interface, not the implementation
// ============================================================

class order_client {
    i_order_service& service_;
public:
    explicit order_client(i_order_service& svc) : service_(svc) {}

    void fetch_and_print(int id) {
        auto result = service_.get_order(id);
        std::visit([](const auto& r) {
            using T = std::decay_t<decltype(r)>;
            if constexpr (std::is_same_v<T, order_dto>) {
                std::cout << "  Order #" << r.id
                          << " customer=" << r.customer
                          << " amount=" << r.amount
                          << " status=" << r.status << "\n";
            } else {
                std::cout << "  Error " << r.code << ": " << r.message << "\n";
            }
        }, result);
    }

    std::optional<int> place_order(const std::string& customer, double amount) {
        auto result = service_.place_order(customer, amount);
        if (auto* o = std::get_if<order_dto>(&result)) {
            std::cout << "  Placed order #" << o->id << " for " << customer << "\n";
            return o->id;
        }
        if (auto* e = std::get_if<service_error>(&result)) {
            std::cout << "  Failed to place order: " << e->message << "\n";
        }
        return std::nullopt;
    }
};

// ============================================================
// IDEMPOTENT OPERATION — safe to retry
// ============================================================

class idempotent_order_placer {
    i_order_service& service_;
    std::unordered_set<std::string> processed_; // idempotency keys
public:
    explicit idempotent_order_placer(i_order_service& svc) : service_(svc) {}

    std::optional<order_dto> place_once(const std::string& idempotency_key,
                                         const std::string& customer,
                                         double amount) {
        if (processed_.count(idempotency_key)) {
            std::cout << "  [Idempotent] Duplicate request ignored: "
                      << idempotency_key << "\n";
            return std::nullopt;
        }
        auto result = service_.place_order(customer, amount);
        if (auto* o = std::get_if<order_dto>(&result)) {
            processed_.insert(idempotency_key);
            return *o;
        }
        return std::nullopt;
    }
};

// ============================================================
// SIMPLE MESSAGE BUS (pub/sub — broker topology)
// ============================================================

struct message {
    std::string topic;
    std::string payload;
};

class message_bus {
    std::unordered_map<std::string,
        std::vector<std::function<void(const message&)>>> subscribers_;
public:
    void subscribe(const std::string& topic,
                   std::function<void(const message&)> handler) {
        subscribers_[topic].push_back(std::move(handler));
    }
    void publish(const std::string& topic, const std::string& payload) {
        message msg{topic, payload};
        auto it = subscribers_.find(topic);
        if (it != subscribers_.end())
            for (const auto& h : it->second) h(msg);
    }
};

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== SOA Demo ===\n\n";

    // --- Service client ---
    std::cout << "--- Service Interface + Client ---\n";
    local_order_service svc;
    order_client client(svc);

    auto id1 = client.place_order("Alice", 149.99);
    auto id2 = client.place_order("Bob", -10.0); // invalid
    if (id1) client.fetch_and_print(*id1);
    client.fetch_and_print(9999); // not found

    // --- Idempotent placement ---
    std::cout << "\n--- Idempotent Operation ---\n";
    idempotent_order_placer iplacer(svc);
    iplacer.place_once("req-001", "Carol", 50.0);
    iplacer.place_once("req-001", "Carol", 50.0); // duplicate — ignored
    iplacer.place_once("req-002", "Dave", 75.0);

    // --- Message bus ---
    std::cout << "\n--- Message Bus (Pub/Sub) ---\n";
    message_bus bus;
    bus.subscribe("orders.placed", [](const message& m) {
        std::cout << "  [Inventory] Reserving stock: " << m.payload << "\n";
    });
    bus.subscribe("orders.placed", [](const message& m) {
        std::cout << "  [Email] Sending confirmation: " << m.payload << "\n";
    });
    bus.subscribe("orders.cancelled", [](const message& m) {
        std::cout << "  [Refund] Processing refund: " << m.payload << "\n";
    });

    bus.publish("orders.placed", "order_id=5 customer=Eve amount=99.99");
    bus.publish("orders.placed", "order_id=6 customer=Frank amount=29.99");
    bus.publish("orders.cancelled", "order_id=5");

    std::cout << "\nDone.\n";
    return 0;
}
