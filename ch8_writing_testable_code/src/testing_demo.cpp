// testing_demo.cpp
//
// Demonstrates:
//   - Dependency injection enabling testability
//   - Test doubles (stub, fake, mock)
//   - A lightweight inline test runner (no external framework)
//
// Book reference: Chapter 8, §8.3-8.4 (Mocks/fakes, TDD design)

#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================
// LIGHTWEIGHT TEST RUNNER (no external framework)
// ============================================================

int g_pass_count = 0;
int g_fail_count = 0;

#define EXPECT_EQ(actual, expected)                                     \
    do {                                                                 \
        if ((actual) == (expected)) {                                    \
            ++g_pass_count;                                              \
        } else {                                                         \
            ++g_fail_count;                                              \
            std::cout << "  FAIL [" << __FILE__ << ":" << __LINE__ << "] " \
                      << #actual << " == " << #expected << "\n";         \
            std::cout << "       actual:   " << (actual) << "\n";        \
            std::cout << "       expected: " << (expected) << "\n";      \
        }                                                                \
    } while (false)

#define EXPECT_TRUE(cond)                                               \
    do {                                                                 \
        if (cond) {                                                      \
            ++g_pass_count;                                              \
        } else {                                                         \
            ++g_fail_count;                                              \
            std::cout << "  FAIL [" << __FILE__ << ":" << __LINE__ << "] " \
                      << #cond << " is false\n";                         \
        }                                                                \
    } while (false)

void run_test(const std::string& name, std::function<void()> test_fn) {
    std::cout << "[ RUN ] " << name << "\n";
    test_fn();
    std::cout << "[  OK ] " << name << "\n";
}

// ============================================================
// PRODUCTION CODE — designed for testability
// ============================================================

// Interface (abstraction) — allows test doubles to be injected
class i_email_service {
public:
    virtual void send(const std::string& to,
                      const std::string& subject,
                      const std::string& body) = 0;
    virtual ~i_email_service() = default;
};

class i_order_repository {
public:
    virtual void save(int order_id, double amount) = 0;
    virtual std::optional<double> find(int order_id) = 0;
    virtual ~i_order_repository() = default;
};

class pricing_service {
    double tax_rate_;
public:
    explicit pricing_service(double tax_rate) : tax_rate_(tax_rate) {}
    double apply_tax(double amount) const {
        if (amount < 0) throw std::invalid_argument("amount must be non-negative");
        return amount * (1.0 + tax_rate_);
    }
    double tax_rate() const { return tax_rate_; }
};

// High-level service — depends on abstractions (DIP)
class order_service {
    i_order_repository& repo_;
    i_email_service&    email_;
    pricing_service&    pricer_;
public:
    order_service(i_order_repository& repo,
                  i_email_service&    email,
                  pricing_service&    pricer)
        : repo_(repo), email_(email), pricer_(pricer) {}

    void place_order(int order_id, const std::string& customer_email, double amount) {
        double total = pricer_.apply_tax(amount);
        repo_.save(order_id, total);
        email_.send(customer_email, "Order Confirmed",
                    "Your order #" + std::to_string(order_id) +
                    " total: " + std::to_string(total));
    }
    std::optional<double> get_order_total(int order_id) {
        return repo_.find(order_id);
    }
};

// ============================================================
// TEST DOUBLES
// ============================================================

// FAKE: simple in-memory repository
class in_memory_order_repository : public i_order_repository {
    std::vector<std::pair<int, double>> orders_;
public:
    void save(int order_id, double amount) override {
        orders_.emplace_back(order_id, amount);
    }
    std::optional<double> find(int order_id) override {
        for (const auto& [id, amount] : orders_)
            if (id == order_id) return amount;
        return std::nullopt;
    }
    size_t size() const { return orders_.size(); }
};

// SPY/MOCK: records interactions for assertion
class spy_email_service : public i_email_service {
public:
    struct call {
        std::string to;
        std::string subject;
        std::string body;
    };
    std::vector<call> calls;

    void send(const std::string& to, const std::string& subject,
              const std::string& body) override {
        calls.push_back({to, subject, body});
    }
    int call_count() const { return static_cast<int>(calls.size()); }
};

// STUB: always returns a fixed value
class stub_order_repository : public i_order_repository {
public:
    void save(int, double) override {}
    std::optional<double> find(int) override { return 42.0; }
};

// ============================================================
// TESTS
// ============================================================

void test_pricing_service() {
    run_test("PricingService: apply_tax adds correct percentage", [] {
        pricing_service ps(0.2);
        EXPECT_EQ(ps.apply_tax(100.0), 120.0);
    });
    run_test("PricingService: zero amount remains zero", [] {
        pricing_service ps(0.2);
        EXPECT_EQ(ps.apply_tax(0.0), 0.0);
    });
    run_test("PricingService: negative amount throws", [] {
        pricing_service ps(0.2);
        try {
            ps.apply_tax(-1.0);
            ++g_fail_count;
            std::cout << "  FAIL: expected exception not thrown\n";
        } catch (const std::invalid_argument&) {
            ++g_pass_count;
        }
    });
}

void test_order_service_with_fake() {
    run_test("OrderService: place_order saves order with correct total", [] {
        in_memory_order_repository repo;
        spy_email_service          email;
        pricing_service            pricer(0.2);
        order_service              svc(repo, email, pricer);

        svc.place_order(1, "alice@example.com", 100.0);

        auto total = repo.find(1);
        EXPECT_TRUE(total.has_value());
        EXPECT_EQ(*total, 120.0); // 100 * 1.2
    });

    run_test("OrderService: place_order sends exactly one email", [] {
        in_memory_order_repository repo;
        spy_email_service          email;
        pricing_service            pricer(0.2);
        order_service              svc(repo, email, pricer);

        svc.place_order(1, "alice@example.com", 100.0);

        EXPECT_EQ(email.call_count(), 1);
        EXPECT_EQ(email.calls[0].to, "alice@example.com");
        EXPECT_EQ(email.calls[0].subject, "Order Confirmed");
    });

    run_test("OrderService: get_order_total returns saved amount", [] {
        in_memory_order_repository repo;
        spy_email_service          email;
        pricing_service            pricer(0.0); // no tax for simplicity
        order_service              svc(repo, email, pricer);

        svc.place_order(42, "bob@example.com", 75.0);
        auto total = svc.get_order_total(42);

        EXPECT_TRUE(total.has_value());
        EXPECT_EQ(*total, 75.0);
    });

    run_test("OrderService: unknown order returns nullopt", [] {
        in_memory_order_repository repo;
        spy_email_service          email;
        pricing_service            pricer(0.2);
        order_service              svc(repo, email, pricer);

        auto total = svc.get_order_total(9999);
        EXPECT_TRUE(!total.has_value());
    });
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== Testing Demo ===\n\n";

    test_pricing_service();
    test_order_service_with_fake();

    std::cout << "\n=== Results: "
              << g_pass_count << " passed, "
              << g_fail_count << " failed ===\n";

    return g_fail_count > 0 ? 1 : 0;
}
