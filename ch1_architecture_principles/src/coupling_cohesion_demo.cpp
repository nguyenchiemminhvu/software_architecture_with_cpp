// coupling_cohesion_demo.cpp
//
// Illustrates the spectrum from high-coupling/low-cohesion to
// low-coupling/high-cohesion using order processing as a domain example.
//
// Book reference: Chapter 1, §1.7 (Coupling and cohesion)

#include <iostream>
#include <string>
#include <vector>
#include <functional>

// ============================================================
// BAD: High coupling, low cohesion
//
// order_blob knows about everything and is responsible for nothing specific.
// Any change to email, DB, or pricing forces recompiling this whole unit.
// ============================================================
namespace bad {
struct order_blob {
    int    id;
    double amount;
    std::string customer_email;

    // Pricing, persistence, notification all tangled together
    double compute_total_with_tax() {
        return amount * 1.2;  // hard-coded tax rate — magic constant
    }
    void save_to_db() {
        std::cout << "[bad] DB save order " << id << " amount " << amount << "\n";
    }
    void send_confirmation() {
        std::cout << "[bad] Email to " << customer_email
                  << ": order " << id << " confirmed\n";
    }
    void process() {
        double total = compute_total_with_tax();
        save_to_db();
        send_confirmation();
        std::cout << "[bad] Order " << id << " processed. Total: " << total << "\n";
    }
};
} // namespace bad

// ============================================================
// GOOD: Low coupling, high cohesion
//
// Each class has one job. They communicate through minimal interfaces
// (data coupling — passing only what is needed).
// ============================================================
namespace good {

// Cohesive: responsible ONLY for pricing rules
class pricing_service {
    double tax_rate_;
public:
    explicit pricing_service(double tax_rate) : tax_rate_(tax_rate) {}
    double apply_tax(double amount) const { return amount * (1.0 + tax_rate_); }
};

// Cohesive: responsible ONLY for persistence
class order_repository {
public:
    void save(int order_id, double total) {
        std::cout << "[good] DB save order " << order_id
                  << " total " << total << "\n";
    }
};

// Cohesive: responsible ONLY for notifications
class notification_service {
public:
    void send_confirmation(const std::string& email, int order_id) {
        std::cout << "[good] Email to " << email
                  << ": order " << order_id << " confirmed\n";
    }
};

struct order {
    int         id;
    double      amount;
    std::string customer_email;
};

// High-level orchestrator — depends on abstractions, passes only what each service needs
class order_processor {
    pricing_service&      pricer_;
    order_repository&     repo_;
    notification_service& notifier_;
public:
    order_processor(pricing_service& p, order_repository& r, notification_service& n)
        : pricer_(p), repo_(r), notifier_(n) {}

    void process(const order& o) {
        double total = pricer_.apply_tax(o.amount);
        repo_.save(o.id, total);
        notifier_.send_confirmation(o.customer_email, o.id);
        std::cout << "[good] Order " << o.id << " processed. Total: " << total << "\n";
    }
};
} // namespace good

// ============================================================
// Demonstrating data coupling vs stamp coupling
// ============================================================
namespace coupling_types {

struct full_user {
    int         id;
    std::string name;
    std::string email;
    std::string address;
    int         age;
};

// STAMP coupling — passes more than needed
void send_email_stamp(const full_user& u) {
    std::cout << "[stamp coupling] Sending to " << u.email << "\n";
}

// DATA coupling — passes only what is needed
void send_email_data(const std::string& email) {
    std::cout << "[data coupling] Sending to " << email << "\n";
}

} // namespace coupling_types

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== Coupling & Cohesion Demo ===\n\n";

    std::cout << "--- Bad: High Coupling, Low Cohesion ---\n";
    bad::order_blob blob{1, 100.0, "alice@example.com"};
    blob.process();

    std::cout << "\n--- Good: Low Coupling, High Cohesion ---\n";
    good::pricing_service      pricer(0.2);
    good::order_repository     repo;
    good::notification_service notifier;
    good::order_processor      processor(pricer, repo, notifier);

    good::order o{2, 100.0, "bob@example.com"};
    processor.process(o);

    std::cout << "\n--- Coupling Types ---\n";
    coupling_types::full_user user{1, "Carol", "carol@example.com", "123 Main St", 30};
    coupling_types::send_email_stamp(user);   // passes full struct
    coupling_types::send_email_data(user.email); // passes only what is needed

    std::cout << "\nDone.\n";
    return 0;
}
