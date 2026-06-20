# Workshop Slides: Software Architecture with C++
## Based on Ostrowski & Gaczkowski — Packt, 2021

---

## Slide 1 — Title

# Software Architecture with C++

**Building scalable and sustainable enterprise software**

Ostrowski & Gaczkowski · Packt Publishing · 2021

*This workshop is a structured study guide through all 15 chapters*

---

## Slide 2 — Who This Workshop Is For

- C++ developers moving into technical leadership
- Architects who want to anchor decisions in C++ idioms
- Engineers working on distributed systems in C++
- Anyone who needs to communicate architecture across teams

**Prerequisites**: Proficiency in C++14/17; basic familiarity with CMake

---

## Slide 3 — Workshop Structure

```
Part I   (Ch 1–3)   Architecture Fundamentals
Part II  (Ch 4–8)   Design and Development
Part III (Ch 9–11)  Quality Attributes
Part IV  (Ch 12–15) Cloud-Native Systems
```

Each chapter = **concept lecture + C++ demo + discussion**

---

## Slide 4 — Learning Goals

After this workshop you will be able to:

1. Apply SOLID and DDD to decompose large C++ systems
2. Choose between architectural styles for a given problem
3. Use C++20 features for safer, more expressive design
4. Write testable C++ code with dependency injection
5. Design secure, high-performance services
6. Architect and operate microservices in Kubernetes

---

# PART I — ARCHITECTURE FUNDAMENTALS

---

## Slide 5 — Ch 1: Principles of Software Architecture

**Core question**: What separates *design* from *architecture*?

Architecture = decisions that are **hard to reverse** and have **systemic impact**

Key principles:
- **SRP** — one reason to change
- **OCP** — open to extension, closed to modification
- **DIP** — depend on abstractions, not concretions
- **DRY** — every piece of knowledge has one authoritative source
- **YAGNI** — don't build what you don't need today

---

## Slide 6 — Coupling and Cohesion

```
High Cohesion + Low Coupling = the architectural ideal
```

| | Low Cohesion | High Cohesion |
|---|---|---|
| **Low Coupling** | ✅ Ideal | ⚠️ Fragmented |
| **High Coupling** | ❌ God module | ❌ Worst case |

**C++ idiom**: Use interfaces (pure virtual) to invert dependencies

```cpp
class i_logger { public: virtual void log(std::string_view) = 0; };
class order_service {
    i_logger& log_;  // depends on abstraction
public:
    order_service(i_logger& l) : log_(l) {}
};
```

---

## Slide 7 — Domain-Driven Design in C++

DDD tactical patterns map naturally to C++ types:

| DDD Concept | C++ Idiom |
|---|---|
| Entity | Class with identity field |
| Value Object | `const` struct, `operator==` on fields |
| Aggregate | Class owning a consistency boundary |
| Repository | Abstract class returning domain objects |
| Domain Event | `struct OrderPlaced { ... }` |

---

## Slide 8 — Ch 2: Architectural Styles

**Key question**: What trade-offs does each style impose?

| Style | Coupling | Scalability | Complexity |
|---|---|---|---|
| Monolith | Low ops complexity | Vertical | Low initially |
| Layered | Moderate | Vertical | Moderate |
| Event-driven | Very loose | High | High (async) |
| Microservices | Loose | Horizontal | Highest |

**Choose the simplest style that meets your scalability and team requirements.**

---

## Slide 9 — Event-Driven Architecture in C++

A typed event bus decouples publishers from subscribers:

```cpp
template<typename Event>
class event_bus {
    std::vector<std::function<void(const Event&)>> handlers_;
public:
    void subscribe(std::function<void(const Event&)> h) { handlers_.push_back(h); }
    void publish(const Event& e) { for (auto& h : handlers_) h(e); }
};
```

Benefits: zero source-code dependencies between subsystems.

---

## Slide 10 — Ch 3: Requirements

Architecturally Significant Requirements (ASRs) drive structure:

- **Functional**: what the system does
- **Non-functional (quality attributes)**: how well it does it

Quality attributes that most often drive architectural decisions:
- Availability, Performance, Security, Maintainability, Scalability

**4+1 View Model**: Logical · Development · Process · Physical · Scenarios

---

# PART II — DESIGN AND DEVELOPMENT

---

## Slide 11 — Ch 4: System Design Challenges

**Eight Fallacies of Distributed Computing** (Deutsch, 1994):

1. The network is reliable
2. Latency is zero
3. Bandwidth is infinite
4. The network is secure
5. Topology doesn't change
6. There is one administrator
7. Transport cost is zero
8. The network is homogeneous

*Every one of these will eventually be false in production.*

---

## Slide 12 — CAP Theorem

A distributed system can guarantee at most **two of three**:

```
      Consistency
         / \
        /   \
       /     \
  CA  /       \  CP
     /         \
    /    AP     \
Availability -- Partition Tolerance
```

- **CA** — impossible in practice (partitions always happen)
- **CP** — choose consistency (HBase, ZooKeeper)
- **AP** — choose availability (DynamoDB, Cassandra in default mode)

---

## Slide 13 — CQRS Pattern in C++

Separate read and write models for complex domains:

```cpp
// Write side
class place_order_command { /* validation logic */ };

// Read side
class order_summary_query { /* optimised projection */ };

// Event bridge
// PlaceOrder → Order aggregate → OrderPlaced event → read model update
```

Benefits: independent scaling; optimise each side for its access pattern.

---

## Slide 14 — Ch 5: C++ Language Features for Architecture

C++20 features that matter for architectural code:

| Feature | Architectural Use |
|---|---|
| `std::optional<T>` | Explicit nullable types; no null pointer bugs |
| `std::span<T>` | Non-owning views; eliminate pointer + length pairs |
| `std::variant<...>` | Sum types; replace inheritance for closed sets |
| Concepts | Constrain template parameters; better error messages |
| `constexpr` | Move validation to compile time |
| Ranges | Pipeline-style data transformations; composable |

---

## Slide 15 — Strong Types

```cpp
// Weak types: two ints in wrong order → silent bug
void create_account(int user_id, int account_id);

// Strong types: compile-time protection
using user_id_t    = int;  // Still weak — use a strong wrapper
struct user_id    { int value; };
struct account_id { int value; };
void create_account(user_id, account_id); // safe
```

Use `[[nodiscard]]` on functions returning error codes or results.

---

## Slide 16 — Ch 6: Design Patterns in C++

Classic patterns reinterpreted for modern C++:

| Pattern | Modern C++ Idiom |
|---|---|
| RAII Guard | `std::lock_guard`, custom `scope_exit` |
| Factory | `std::function` returning `std::unique_ptr<Base>` |
| Observer | `std::function` subscriber list |
| Strategy | Policy template parameter (zero-overhead) |
| CRTP | Static polymorphism for performance-critical paths |

---

## Slide 17 — CRTP: Static Polymorphism

```cpp
template<typename Derived>
struct serialisable {
    std::string serialise() const {
        return static_cast<const Derived*>(this)->do_serialise();
    }
};

struct order : serialisable<order> {
    std::string do_serialise() const { return "Order{...}"; }
};
```

Zero virtual dispatch overhead. Used in STL (e.g., `std::char_traits`).

---

## Slide 18 — Ch 7: Building and Packaging

**CMake modern idioms** (CMake ≥ 3.16):

```cmake
add_library(order_lib STATIC src/order.cpp)
target_include_directories(order_lib PUBLIC include/)
target_compile_features(order_lib PUBLIC cxx_std_20)

add_executable(order_service src/main.cpp)
target_link_libraries(order_service PRIVATE order_lib)
```

**Conan 2** for dependency management:
```bash
conan install . --build=missing
cmake --preset conan-release
```

---

## Slide 19 — Static Analysis

Integrate into CI, not just local development:

| Tool | What it catches |
|---|---|
| `clang-tidy` | Style, modernisation, bug-prone patterns |
| `clang-format` | Formatting (zero diff noise in PRs) |
| `AddressSanitizer` | Memory errors (buffer overflow, UAF) |
| `UBSanitizer` | Undefined behaviour |
| `ThreadSanitizer` | Data races |
| `cppcheck` | Additional static checks |

---

# PART III — QUALITY ATTRIBUTES

---

## Slide 20 — Ch 8: Testable Code

**Testing pyramid** for C++ services:

```
        /   E2E   \       few, slow, brittle
       /  Integration \   more, medium speed
      /  Unit Tests    \  many, fast, isolated
```

Key: design for testability from day one.

---

## Slide 21 — Dependency Injection in C++

```cpp
class order_service {
    i_payment_gateway& payment_;
    i_inventory&       inventory_;
public:
    order_service(i_payment_gateway& p, i_inventory& i)
        : payment_(p), inventory_(i) {}
    // test: inject fake_payment, fake_inventory
};
```

No dependency injection framework needed in C++. Constructor injection is sufficient.

---

## Slide 22 — Ch 9: CI/CD

**CI pipeline stages**:

```
git push → build → test → lint → package → deploy to staging → integration test → production
```

**Key practices**:
- Every commit builds and tests
- Build once, deploy the same artefact everywhere
- Feature flags over long-lived branches
- Automated rollback on health probe failure
- Trunk-based development (no release branches in CD)

---

## Slide 23 — Ch 10: Security

**OWASP Top 10** — know the list. Most common C++ surface areas:

- **A01 Broken Access Control** — check permissions at every layer
- **A02 Cryptographic Failures** — use system TLS; never roll your own crypto
- **A03 Injection** — validate and sanitise all external input
- **A05 Security Misconfiguration** — no debug endpoints, no default passwords
- **A09 Logging Failures** — log security events; do not log secrets

---

## Slide 24 — Secure C++ Idioms

```cpp
// ❌ Buffer overflow risk
void process(char* data, int len);

// ✅ Use std::span — bounds-checked view
void process(std::span<const std::byte> data) {
    if (data.size() > MAX_PAYLOAD) throw std::invalid_argument{"too large"};
}

// ✅ Signed arithmetic — use checked arithmetic or safeint
auto safe_add(int a, int b) -> std::optional<int> {
    if (b > 0 && a > INT_MAX - b) return std::nullopt;
    return a + b;
}
```

---

## Slide 25 — Ch 11: Performance

**Performance workflow**:

```
Measure → Profile → Identify hotspot → Optimise → Measure again
```

Never optimise without measurement. Always benchmark before and after.

**Tools**: `perf` (Linux), Instruments (macOS), `valgrind --tool=callgrind`, Google Benchmark

---

## Slide 26 — Cache-Friendly Design

Cache line = 64 bytes. Design data to minimise cache misses:

```cpp
// AoS (Array of Structs) — interleaves hot and cold data
struct particle { float x, y, z;  float vx, vy, vz;  float mass; float health; };

// SoA (Struct of Arrays) — hot data together
struct particles {
    std::vector<float> x, y, z;    // position — hot
    std::vector<float> vx, vy, vz; // velocity — hot
    std::vector<float> mass;        // cold
    std::vector<float> health;      // cold
};
```

For tight physics loops, SoA can be 2–4× faster.

---

# PART IV — CLOUD-NATIVE

---

## Slide 27 — Ch 12: Service-Oriented Architecture

**SOA principles**:
- Services have explicit, versioned contracts
- Services are independently deployable
- Communication over standard protocols

**Choosing a protocol**:

| Protocol | Use case |
|---|---|
| REST/HTTP | Public APIs, simple CRUD |
| gRPC | Internal services, streaming, strong typing |
| MQTT | IoT, low-bandwidth pub/sub |
| ZeroMQ | High-throughput, broker-free messaging |
| Kafka | High-volume event streaming, replay |

---

## Slide 28 — REST vs gRPC

| | REST | gRPC |
|---|---|---|
| Serialisation | JSON (text) | Protobuf (binary) |
| Schema | Optional (OpenAPI) | Required (.proto) |
| Streaming | Limited (SSE/WebSocket) | Native (4 modes) |
| Browser support | Native | Via grpc-web |
| Performance | Baseline | ~5–10× faster |
| Type safety | Runtime | Compile-time |

*Use REST for external-facing APIs; gRPC for internal service-to-service calls.*

---

## Slide 29 — Ch 13: Microservices

**Microservice decomposition by bounded context**:

```
Identify bounded contexts (DDD) → one service per context
Each service owns its data → no shared databases
Communicate via events → loose temporal coupling
```

Anti-patterns to avoid:
- **Distributed monolith** — micro deployments, tight coupling
- **Shared DB** — breaks independent deployment
- **Nanoservices** — overhead exceeds benefit

---

## Slide 30 — The Saga Pattern

Distributed transactions without 2PC:

```
OrderPlaced
  └→ PaymentService.ChargeCard
       ├ success → InventoryService.ReserveStock
       │               ├ success → Order.Confirm
       │               └ failure → PaymentService.RefundCard
       │                           → Order.Cancel
       └ failure → Order.Cancel
```

Each step has a **compensating transaction**.

---

## Slide 31 — Observability

The three pillars:

```
Logs     — what happened (structured JSON, correlation ID)
Metrics  — how the system performs (RED: Rate, Errors, Duration)
Traces   — how a request flowed through services (OpenTelemetry)
```

**Correlation ID**: generated at the edge; propagated in every downstream call header.

---

## Slide 32 — Ch 14: Containers

**Multi-stage Dockerfile for C++**:

```dockerfile
FROM ubuntu:22.04 AS builder
RUN apt-get install -y cmake g++
COPY . .
RUN cmake -B build && cmake --build build -j$(nproc)

FROM ubuntu:22.04 AS runtime
COPY --from=builder /build/my_service /app/
USER nobody
ENTRYPOINT ["/app/my_service"]
```

Result: build image (~2 GB) vs runtime image (~150 MB).

---

## Slide 33 — Kubernetes Essentials

**Resources every service needs**:

```yaml
resources:
  requests: {cpu: "100m", memory: "128Mi"}  # scheduling
  limits:   {cpu: "500m", memory: "512Mi"}  # enforcement

readinessProbe:
  httpGet: {path: /health, port: 8080}      # load balancing gate

livenessProbe:
  httpGet: {path: /health, port: 8080}      # restart gate
```

Never deploy without all three.

---

## Slide 34 — Ch 15: Cloud-Native Design

**Twelve-Factor App** — most critical factors:

| Factor | Key Rule |
|---|---|
| III. Config | No config in code; use env vars / secrets |
| VI. Processes | Stateless; state in backing services |
| IX. Disposability | Fast start, graceful SIGTERM handling |
| XI. Logs | `stdout` only; never write log files |

---

## Slide 35 — Cloud-Native Resilience Patterns

**Circuit Breaker**:
```
Closed → failure threshold → Open → timeout → Half-Open → probe success → Closed
```

**Bulkhead**: separate thread pools per dependency

**Retry with exponential backoff and jitter**:
```
wait = min(base * 2^attempt + jitter, max_wait)
```

---

## Slide 36 — Service Mesh

Move cross-cutting concerns to infrastructure:

```
Without mesh: app code handles TLS, retries, timeouts, tracing headers
With mesh:    sidecar proxy handles TLS, retries, timeouts, tracing
              → app code is pure business logic
```

Trade-off: operational complexity of mesh vs simplicity of application code.

---

## Slide 37 — Summary and Key Takeaways

1. **Architecture is about trade-offs** — there is no universally best style
2. **Design for change** — SOLID, DDD, interfaces
3. **C++20 is expressive and safe** — use `optional`, `span`, `variant`, concepts
4. **Testability is designed in** — dependency injection, interfaces, seams
5. **Measure before optimising** — profile-guided, cache-aware design
6. **Secure by design** — validate input, least privilege, no magic crypto
7. **Observability is mandatory** — logs + metrics + traces from day one
8. **Microservices are a trade-off** — start simpler; split when seams are clear

---

## Slide 38 — Recommended Next Steps

- Read [*Designing Data-Intensive Applications*](https://dataintensive.net/) (Kleppmann)
- Study [*Clean Architecture*](https://www.oreilly.com/library/view/clean-architecture-a/9780134494272/) (Martin)
- Explore [*Large-Scale C++ — Process and Architecture*](https://www.oreilly.com/library/view/large-scale-c-volume/9780133927573/) (Lakos)
- Practice: architect a real system; write ADRs for every major decision
- Tools: OpenTelemetry, Istio, Conan 2, clang-tidy, Google Benchmark
