# Chapter 5: Leveraging C++ Language Features

**Book Pages**: 133–164 | *Software Architecture with C++* by Ostrowski & Gaczkowski

---

## Why This Chapter Matters

C++ gives architects and developers a uniquely powerful toolbox. Using language features
correctly results in code that is safer, faster, more expressive, and easier to maintain.
This chapter surveys the features most impactful at the architectural level.

---

## 5.1 Designing Great APIs

A great API is:
- **Hard to misuse**: incorrect usage should be a compile-time error, not a runtime crash
- **Minimal**: exposes only what is necessary
- **Consistent**: follows naming and style conventions throughout
- **Efficient**: zero-cost abstractions where possible

---

## 5.2 RAII — Resource Acquisition Is Initialisation

RAII is C++'s most important idiom. Resources (memory, files, locks, sockets) are tied to
object lifetime. When the object goes out of scope, the destructor releases the resource.

```cpp
// WITHOUT RAII — fragile, exception-unsafe
FILE* f = fopen("data.bin", "rb");
// ... if exception here: file leaked
fclose(f);

// WITH RAII — exception-safe, no manual cleanup
class file_guard {
    FILE* f_;
public:
    explicit file_guard(const char* path) : f_(fopen(path, "rb")) {
        if (!f_) throw std::runtime_error("cannot open file");
    }
    ~file_guard() { if (f_) fclose(f_); }
    FILE* get() const { return f_; }
    // Non-copyable
    file_guard(const file_guard&) = delete;
    file_guard& operator=(const file_guard&) = delete;
};
```

All standard library resource types use RAII: `std::unique_ptr`, `std::shared_ptr`,
`std::lock_guard`, `std::fstream`, `std::vector`.

---

## 5.3 Using `std::optional`

`std::optional<T>` explicitly represents a value that may or may not exist. It eliminates
sentinel values (−1, null pointer, empty string) that silently propagate errors.

```cpp
// FRAGILE: uses -1 as sentinel
int find_user_id(const std::string& name);  // returns -1 if not found
// Caller must remember to check; -1 is easily mistaken for a valid ID

// SAFE: intent is explicit in the type
std::optional<int> find_user_id(const std::string& name);
// Caller MUST unwrap — cannot use int directly
auto id = find_user_id("Alice");
if (id) {
    process_user(*id);
}
```

### Optional as Function Parameter

Prefer overloads over optional parameters for clearer API:
```cpp
// AVOID: caller must know what null means
void connect(const std::string& host, std::optional<int> port = std::nullopt);

// PREFER: explicit overloads
void connect(const std::string& host);
void connect(const std::string& host, int port);
```

---

## 5.4 Declarative Code with Ranges (C++20)

Ranges allow expressing data transformations as pipelines — readable, composable, lazy:

```cpp
#include <ranges>
#include <vector>
#include <algorithm>
#include <iostream>

std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Declarative pipeline: filter evens, square them, take first 3
auto result = numbers
    | std::views::filter([](int n) { return n % 2 == 0; })
    | std::views::transform([](int n) { return n * n; })
    | std::views::take(3);

for (int v : result) std::cout << v << " "; // 4 16 36
```

Benefits:
- Lazy evaluation: elements processed only when consumed
- No intermediate containers allocated
- Self-documenting pipeline

---

## 5.5 Moving Computations to Compile Time

`constexpr` and `consteval` move computation from runtime to compile time:

```cpp
// Computed at compile time — zero runtime cost
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}
static_assert(factorial(10) == 3628800, "compile-time check");

// C++20: if consteval — expression context
consteval int must_be_compile_time(int n) { return n * 2; }
```

Use cases:
- Configuration constants
- Hash table sizes (next prime at compile time)
- Protocol message format definitions
- Mathematical tables (sin/cos lookup)

---

## 5.6 Strong Types — Making Interfaces Hard to Misuse

Primitive types carry no semantic information. Strong types make misuse a compiler error:

```cpp
// BAD: easy to pass arguments in wrong order
void set_window_size(int width, int height);
set_window_size(1080, 1920); // compiles, but WRONG

// GOOD: named types prevent argument swapping
struct width_t  { int value; };
struct height_t { int value; };
void set_window_size(width_t w, height_t h);
set_window_size(width_t{1920}, height_t{1080}); // correct
// set_window_size(height_t{1080}, width_t{1920}); // compile error
```

### `enum class` for Flags

```cpp
// BAD: open to misuse
void set_mode(int mode); // 0, 1, 2 — what do they mean?

// GOOD: scoped enum
enum class render_mode { wireframe, solid, textured };
void set_mode(render_mode mode);
set_mode(render_mode::solid);
```

---

## 5.7 Constraining Templates with Concepts (C++20)

C++20 concepts turn template errors from cryptic gibberish into clear diagnostic messages:

```cpp
// Without concepts: error message 3 pages long
template<typename T>
void sort_range(T& container) { std::sort(container.begin(), container.end()); }

// With concepts: self-documenting + clear errors
template<std::ranges::random_access_range T>
    requires std::sortable<std::ranges::iterator_t<T>>
void sort_range(T& container) {
    std::sort(container.begin(), container.end());
}
// sort_range("hello"); → clean error: "string is not a random_access_range"
```

---

## 5.8 Modular C++ (C++20 Modules)

Modules replace the textual inclusion model with a proper module system:

```cpp
// math_utils.cppm — module interface unit
export module math_utils;

export int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}
// Only exported symbols are visible to importers
// No header guards needed; no re-parsing on every include

// main.cpp
import math_utils;
int main() { return factorial(5); }
```

Benefits:
- Faster builds (no repeated header parsing)
- No macro leakage across module boundaries
- Explicit export of API surface

---

## Common Mistakes / Anti-Patterns

| Anti-Pattern | C++ Manifestation | Fix |
|---|---|---|
| **Naked new/delete** | Manual memory management | `std::unique_ptr` / `std::make_unique` |
| **Overloading on pointer + bool** | `void f(const char*)` and `void f(bool)` — implicit conversions surprise | Use explicit overloads or `std::string_view` |
| **Sentinel values** | `-1` or `nullptr` as "no value" | `std::optional<T>` |
| **Boolean parameters** | `send(msg, true, false, 3)` | `enum class` + named types |
| **Fat template errors** | 50-line error for wrong type | C++20 concepts |
| **Include everything** | `#include <bits/stdc++.h>` | Minimal includes; use forward declarations |

---

## Key Takeaways

1. **RAII is non-negotiable** — every resource must have an owning RAII wrapper
2. **`std::optional` eliminates sentinels** — the type system enforces null-checks
3. **Ranges make data transformations readable and efficient** — adopt in C++20 codebases
4. **Compile-time computation is free** — use `constexpr` aggressively
5. **Strong types make APIs hard to misuse** — encode semantics in the type system
6. **Concepts document constraints** — and produce comprehensible errors when violated
