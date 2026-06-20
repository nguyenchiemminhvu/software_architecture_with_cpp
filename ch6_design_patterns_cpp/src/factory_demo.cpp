// factory_demo.cpp
//
// Demonstrates:
//   - Factory method pattern with unique_ptr
//   - Abstract factory
//   - Builder pattern (fluent API)
//
// Book reference: Chapter 6, §6.4 (Creating objects — using factories)

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

// ============================================================
// 1. FACTORY METHOD — returns unique_ptr (clear ownership)
// ============================================================

class shape {
public:
    virtual double area() const = 0;
    virtual std::string_view name() const = 0;
    virtual ~shape() = default;
};

class circle : public shape {
    double radius_;
public:
    explicit circle(double r) : radius_(r) {}
    double area() const override { return 3.14159 * radius_ * radius_; }
    std::string_view name() const override { return "Circle"; }
};

class rectangle : public shape {
    double w_, h_;
public:
    rectangle(double w, double h) : w_(w), h_(h) {}
    double area() const override { return w_ * h_; }
    std::string_view name() const override { return "Rectangle"; }
};

class triangle : public shape {
    double base_, height_;
public:
    triangle(double b, double h) : base_(b), height_(h) {}
    double area() const override { return 0.5 * base_ * height_; }
    std::string_view name() const override { return "Triangle"; }
};

// Factory function — returns by unique_ptr; ownership explicit
std::unique_ptr<shape> make_shape(std::string_view type, double a, double b = 0.0) {
    if (type == "circle")    return std::make_unique<circle>(a);
    if (type == "rectangle") return std::make_unique<rectangle>(a, b);
    if (type == "triangle")  return std::make_unique<triangle>(a, b);
    throw std::invalid_argument(std::string("unknown shape: ") + std::string(type));
}

void demo_factory() {
    std::cout << "--- Factory Method ---\n";
    auto shapes = {
        make_shape("circle",    5.0),
        make_shape("rectangle", 4.0, 6.0),
        make_shape("triangle",  3.0, 8.0),
    };
    for (const auto& s : shapes) {
        std::cout << "  " << s->name() << " area = " << s->area() << "\n";
    }
}

// ============================================================
// 2. ABSTRACT FACTORY — creates families of related objects
// ============================================================

class button {
public:
    virtual void render() const = 0;
    virtual ~button() = default;
};

class checkbox {
public:
    virtual void render() const = 0;
    virtual ~checkbox() = default;
};

class dark_button : public button {
public:
    void render() const override { std::cout << "  [Dark Button]\n"; }
};
class dark_checkbox : public checkbox {
public:
    void render() const override { std::cout << "  [Dark Checkbox]\n"; }
};

class light_button : public button {
public:
    void render() const override { std::cout << "  [Light Button]\n"; }
};
class light_checkbox : public checkbox {
public:
    void render() const override { std::cout << "  [Light Checkbox]\n"; }
};

class ui_factory {
public:
    virtual std::unique_ptr<button>   create_button()   const = 0;
    virtual std::unique_ptr<checkbox> create_checkbox() const = 0;
    virtual ~ui_factory() = default;
};

class dark_theme_factory : public ui_factory {
public:
    std::unique_ptr<button>   create_button()   const override { return std::make_unique<dark_button>(); }
    std::unique_ptr<checkbox> create_checkbox() const override { return std::make_unique<dark_checkbox>(); }
};

class light_theme_factory : public ui_factory {
public:
    std::unique_ptr<button>   create_button()   const override { return std::make_unique<light_button>(); }
    std::unique_ptr<checkbox> create_checkbox() const override { return std::make_unique<light_checkbox>(); }
};

void render_ui(const ui_factory& factory) {
    auto btn = factory.create_button();
    auto chk = factory.create_checkbox();
    btn->render();
    chk->render();
}

void demo_abstract_factory() {
    std::cout << "\n--- Abstract Factory ---\n";
    std::cout << "Dark theme:\n";
    render_ui(dark_theme_factory{});
    std::cout << "Light theme:\n";
    render_ui(light_theme_factory{});
}

// ============================================================
// 3. BUILDER — fluent API for complex construction
// ============================================================

struct database_config {
    std::string host     = "localhost";
    int         port     = 5432;
    std::string database = "default";
    std::string user     = "admin";
    int         pool_size = 5;
    int         timeout_ms = 3000;

    void print() const {
        std::cout << "  host=" << host << " port=" << port
                  << " db=" << database << " user=" << user
                  << " pool=" << pool_size
                  << " timeout=" << timeout_ms << "ms\n";
    }
};

class db_config_builder {
    database_config cfg_;
public:
    db_config_builder& host(std::string h)     { cfg_.host = std::move(h); return *this; }
    db_config_builder& port(int p)             { cfg_.port = p;            return *this; }
    db_config_builder& database(std::string d) { cfg_.database = std::move(d); return *this; }
    db_config_builder& user(std::string u)     { cfg_.user = std::move(u); return *this; }
    db_config_builder& pool_size(int s)        { cfg_.pool_size = s;       return *this; }
    db_config_builder& timeout_ms(int t)       { cfg_.timeout_ms = t;      return *this; }
    database_config build() const { return cfg_; }
};

void demo_builder() {
    std::cout << "\n--- Builder Pattern ---\n";
    auto cfg = db_config_builder{}
        .host("db.prod.example.com")
        .port(5432)
        .database("orders")
        .user("app_user")
        .pool_size(20)
        .timeout_ms(5000)
        .build();
    std::cout << "Production config:\n";
    cfg.print();

    auto test_cfg = db_config_builder{}
        .host("localhost")
        .database("orders_test")
        .build();
    std::cout << "Test config (defaults for most fields):\n";
    test_cfg.print();
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== Factory & Builder Patterns Demo ===\n\n";
    demo_factory();
    demo_abstract_factory();
    demo_builder();
    std::cout << "\nDone.\n";
    return 0;
}
