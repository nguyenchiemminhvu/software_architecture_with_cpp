// optional_demo.cpp
//
// Demonstrates std::optional for safe absence-of-value handling,
// and std::ranges pipelines for declarative data transformations.
//
// Book reference: Chapter 5, §5.3-5.4

#include <algorithm>
#include <iostream>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

// ============================================================
// std::optional — explicit nullable values
// ============================================================

struct user {
    int         id;
    std::string name;
    std::string email;
};

class user_repository {
    std::unordered_map<int, user> users_{
        {1, {1, "Alice", "alice@example.com"}},
        {2, {2, "Bob",   "bob@example.com"}},
    };
public:
    std::optional<user> find_by_id(int id) const {
        auto it = users_.find(id);
        if (it == users_.end()) return std::nullopt;
        return it->second;
    }
    std::optional<user> find_by_name(const std::string& name) const {
        for (const auto& [id, u] : users_)
            if (u.name == name) return u;
        return std::nullopt;
    }
};

// Chaining optionals: get email of user with given name
std::optional<std::string> get_email_for(const user_repository& repo,
                                          const std::string& name) {
    auto user = repo.find_by_name(name);
    if (!user) return std::nullopt;
    return user->email;  // std::optional<std::string>
}

void demo_optional() {
    std::cout << "--- std::optional Demo ---\n";
    user_repository repo;

    // Found
    auto alice = repo.find_by_id(1);
    if (alice) {
        std::cout << "Found: " << alice->name << " <" << alice->email << ">\n";
    }

    // Not found
    auto nobody = repo.find_by_id(99);
    // value_or provides a default without an if statement
    std::cout << "User 99 name: "
              << (nobody ? nobody->name : "(not found)") << "\n";

    // Chained lookup
    auto email = get_email_for(repo, "Bob");
    std::cout << "Bob's email: " << email.value_or("(none)") << "\n";

    auto missing = get_email_for(repo, "Charlie");
    std::cout << "Charlie's email: " << missing.value_or("(none)") << "\n";
}

// ============================================================
// std::ranges pipelines (C++20)
// ============================================================

struct product {
    std::string name;
    double price;
    std::string category;
};

void demo_ranges() {
    std::cout << "\n--- std::ranges Pipeline Demo ---\n";

    std::vector<product> catalogue{
        {"Widget Pro",  29.99, "tools"},
        {"Gadget X",    89.99, "electronics"},
        {"Sprocket",     9.99, "tools"},
        {"MegaTool",   199.99, "tools"},
        {"SmartWidget", 49.99, "electronics"},
        {"Bolt Pack",    4.99, "hardware"},
    };

    // Pipeline: filter tools, sort by price, take top 2, print names
    std::cout << "Top 2 cheapest tools:\n";
    auto tools_sorted = catalogue
        | std::views::filter([](const product& p) {
              return p.category == "tools";
          })
        | std::views::transform([](const product& p) {
              return std::pair<double, std::string>{p.price, p.name};
          });

    // Materialise to sort (ranges::sort needs a range)
    std::vector<std::pair<double, std::string>> tools_vec(
        tools_sorted.begin(), tools_sorted.end());
    std::ranges::sort(tools_vec);

    for (const auto& [price, name] : tools_vec | std::views::take(2)) {
        std::cout << "  " << name << " $" << price << "\n";
    }

    // Sum all electronics prices
    double electronics_total = 0.0;
    for (const auto& p : catalogue
         | std::views::filter([](const product& p){
               return p.category == "electronics"; })) {
        electronics_total += p.price;
    }
    std::cout << "Total electronics value: $" << electronics_total << "\n";

    // Count products under $50
    auto under_50 = std::ranges::count_if(catalogue,
        [](const product& p){ return p.price < 50.0; });
    std::cout << "Products under $50: " << under_50 << "\n";
}

// ============================================================
// constexpr — compile-time computation
// ============================================================

constexpr int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void demo_constexpr() {
    std::cout << "\n--- constexpr Demo ---\n";
    // Computed entirely at compile time
    constexpr int fib10 = fibonacci(10);
    static_assert(fib10 == 55, "fibonacci(10) must be 55");
    std::cout << "fibonacci(10) = " << fib10 << " (computed at compile time)\n";

    // Array size computed at compile time
    constexpr int cache_size = fibonacci(8); // 21
    std::array<int, cache_size> cache{};
    std::cout << "Cache size (fibonacci(8)): " << cache_size << "\n";
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== Optional & Ranges Demo ===\n\n";
    demo_optional();
    demo_ranges();
    demo_constexpr();
    std::cout << "\nDone.\n";
    return 0;
}
