// solid_demo.cpp
//
// Demonstrates each SOLID principle with a before/after example.
//
// Book reference: Chapter 1, §1.5 (Following the SOLID and DRY principles)

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// ============================================================
// 1. SINGLE RESPONSIBILITY PRINCIPLE (SRP)
//    A class has one reason to change.
// ============================================================

// BAD: UserManager does user creation, persistence, emailing, and logging
namespace bad_srp {
class user_manager {
public:
    void create_user(const std::string& name) {
        std::cout << "[BAD SRP] Creating user: " << name << "\n";
        save_to_database(name);
        send_welcome_email(name);
        log_activity("created " + name);
    }
private:
    void save_to_database(const std::string& name) {
        std::cout << "  [DB] Saving " << name << "\n";
    }
    void send_welcome_email(const std::string& name) {
        std::cout << "  [Email] Sending welcome to " << name << "\n";
    }
    void log_activity(const std::string& event) {
        std::cout << "  [Log] " << event << "\n";
    }
};
} // namespace bad_srp

// GOOD: Each class has a single, focused responsibility
namespace good_srp {
class user_repository {
public:
    void save(const std::string& name) {
        std::cout << "  [DB] Saving " << name << "\n";
    }
};

class email_service {
public:
    void send_welcome(const std::string& name) {
        std::cout << "  [Email] Sending welcome to " << name << "\n";
    }
};

class activity_logger {
public:
    void log(const std::string& event) {
        std::cout << "  [Log] " << event << "\n";
    }
};

class user_service {
    user_repository& repo_;
    email_service&   email_;
    activity_logger& logger_;
public:
    user_service(user_repository& repo, email_service& email, activity_logger& logger)
        : repo_(repo), email_(email), logger_(logger) {}

    void create_user(const std::string& name) {
        repo_.save(name);
        email_.send_welcome(name);
        logger_.log("created " + name);
    }
};
} // namespace good_srp

// ============================================================
// 2. OPEN-CLOSED PRINCIPLE (OCP)
//    Open for extension, closed for modification.
// ============================================================
namespace ocp {
// Abstract base — never modified when adding new shapes
class shape {
public:
    virtual double area() const = 0;
    virtual std::string name() const = 0;
    virtual ~shape() = default;
};

class circle : public shape {
    double radius_;
public:
    explicit circle(double r) : radius_(r) {}
    double area() const override { return 3.14159 * radius_ * radius_; }
    std::string name() const override { return "Circle"; }
};

class rectangle : public shape {
    double w_, h_;
public:
    rectangle(double w, double h) : w_(w), h_(h) {}
    double area() const override { return w_ * h_; }
    std::string name() const override { return "Rectangle"; }
};

// Adding triangle requires ZERO modification to existing code
class triangle : public shape {
    double base_, height_;
public:
    triangle(double b, double h) : base_(b), height_(h) {}
    double area() const override { return 0.5 * base_ * height_; }
    std::string name() const override { return "Triangle"; }
};

void print_areas(const std::vector<std::unique_ptr<shape>>& shapes) {
    for (const auto& s : shapes) {
        std::cout << "  " << s->name() << ": " << s->area() << "\n";
    }
}
} // namespace ocp

// ============================================================
// 3. DEPENDENCY INVERSION PRINCIPLE (DIP)
//    Depend on abstractions, not concretions.
// ============================================================
namespace dip {
// Abstraction (interface)
class i_notification_service {
public:
    virtual void send(const std::string& message) = 0;
    virtual ~i_notification_service() = default;
};

// Low-level modules (concretions)
class email_notifier : public i_notification_service {
public:
    void send(const std::string& msg) override {
        std::cout << "  [Email] " << msg << "\n";
    }
};

class sms_notifier : public i_notification_service {
public:
    void send(const std::string& msg) override {
        std::cout << "  [SMS] " << msg << "\n";
    }
};

// High-level module — depends on abstraction, not concretion
class order_processor {
    i_notification_service& notifier_; // injected
public:
    explicit order_processor(i_notification_service& n) : notifier_(n) {}
    void process_order(int order_id) {
        std::cout << "  Processing order #" << order_id << "\n";
        notifier_.send("Order #" + std::to_string(order_id) + " confirmed");
    }
};
} // namespace dip

// ============================================================
// MAIN — run all demos
// ============================================================
int main() {
    std::cout << "=== SOLID Principles Demo ===\n\n";

    // SRP
    std::cout << "--- Single Responsibility Principle ---\n";
    std::cout << "BAD (one class does everything):\n";
    bad_srp::user_manager mgr;
    mgr.create_user("Alice");

    std::cout << "\nGOOD (each class has one job):\n";
    good_srp::user_repository repo;
    good_srp::email_service email;
    good_srp::activity_logger logger;
    good_srp::user_service svc(repo, email, logger);
    svc.create_user("Bob");

    // OCP
    std::cout << "\n--- Open-Closed Principle ---\n";
    std::vector<std::unique_ptr<ocp::shape>> shapes;
    shapes.push_back(std::make_unique<ocp::circle>(5.0));
    shapes.push_back(std::make_unique<ocp::rectangle>(4.0, 6.0));
    shapes.push_back(std::make_unique<ocp::triangle>(3.0, 8.0));
    std::cout << "Areas (no modification to shape/circle/rectangle):\n";
    ocp::print_areas(shapes);

    // DIP
    std::cout << "\n--- Dependency Inversion Principle ---\n";
    dip::email_notifier email_n;
    dip::sms_notifier sms_n;

    std::cout << "Using email notifier:\n";
    dip::order_processor proc1(email_n);
    proc1.process_order(42);

    std::cout << "Swapping to SMS notifier (zero change to order_processor):\n";
    dip::order_processor proc2(sms_n);
    proc2.process_order(43);

    std::cout << "\nDone.\n";
    return 0;
}
