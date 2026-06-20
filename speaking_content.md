# Speaking Content: Software Architecture with C++
## Informal Speaker Notes — Full Workshop

---

### Slide 1 — Title

Welcome everyone. Today we're working through the entire *Software Architecture with C++*
book by Ostrowski and Gaczkowski. This is a practical workshop — every concept we cover
has a corresponding C++ demo we can run and modify. We're not going to just read slides;
we're going to look at actual code.

The goal is that by the end of this workshop, we can make — and defend — architectural decisions
in a C++ codebase, and write code that reflects those decisions clearly.

---

### Slide 2 — Who This Is For

Quick show of hands: who here would describe themselves primarily as a C++ developer?
As an architect? As both?

This material sits at the intersection. If you're a C++ developer who wants to think more
like an architect — this is for you. If you're an architect who wants to understand how
architecture decisions play out in actual C++ code — also for you.

We assume you're comfortable with C++14 or 17, and you've used CMake at least once.
If you haven't, don't worry — the demos are self-contained.

---

### Slide 3 — Workshop Structure

Four parts, roughly half a day each if you go deep on the exercises.

Part One is foundations — why do we care about architecture at all, and what language do
architects use to talk about systems.

Part Two is where the C++ gets interesting — we'll look at how C++20 features directly
support architectural goals, and how to apply design patterns in a modern way.

Part Three is about the quality attributes that actually drive architectural decisions in
practice: testability, security, performance.

Part Four zooms out to cloud-native systems — microservices, containers, Kubernetes.
Some of this is less about C++ specifically and more about the systems your C++ services
live in.

---

### Slide 4 — Learning Goals

I want to be concrete about what "being able to make architectural decisions" means.

It's not about memorising pattern names. It's about being able to answer questions like:
"Should this be two services or one?" "Should this use a shared database or event sourcing?"
"Why is this code slow, and how do I fix it without breaking anything?"

Those questions have answers grounded in principles. That's what we're building today.

---

### Slide 5 — Chapter 1: Principles

Let's start with the most fundamental question: what is architecture, as distinct from design?

The definition I find most useful: architecture is the set of decisions you'd have to reverse
if you chose differently. The things where changing your mind later is expensive.

"Should this class have a constructor or a factory function?" — design, easy to change.
"Should this system be a monolith or microservices?" — architecture, very hard to change.

SOLID is well known, but I want to focus on the ones that matter most at the architectural
level: SRP and DIP.

---

### Slide 6 — Coupling and Cohesion

The coupling/cohesion diagram is perhaps the most important single slide in this entire workshop.

Here's how to think about it practically: when you're reading a class or module and you think
"why are these two things together?", that's low cohesion. When you think "why does this need
to know about that?", that's high coupling.

The C++ idiom for reducing coupling is the interface — a pure virtual base class that defines
what a component needs, not what provides it. This is the Dependency Inversion Principle made
concrete.

```cpp
// order_service doesn't know how logging is implemented
class order_service {
    i_logger& log_;
public:
    order_service(i_logger& l) : log_(l) {}
};
```

In tests, you inject a fake. In production, you inject the real spdlog wrapper. The order
service doesn't care.

---

### Slide 7 — DDD in C++

Domain-Driven Design is about making your code speak the language of the business. When a
domain expert reads your class names and method names, they should recognise them.

Value objects are particularly interesting in C++. A value object has no identity — two
objects with the same fields are equal. In C++ that maps naturally to a simple struct with
`operator==` comparing all fields and no mutable state.

Entities, on the other hand, have identity. Two orders with different IDs are different
even if every other field is identical.

---

### Slide 8 — Chapter 2: Architectural Styles

The most common mistake I see is choosing microservices too early. The book is very honest
about this: microservices have the highest operational complexity. Start with a well-structured
monolith. Split when you have actual evidence that you need to.

Event-driven architecture is interesting because it gives you very low coupling at the cost
of making the system harder to reason about. A function call is easy to trace. An event that
triggers a chain of subscribers is not.

---

### Slide 9 — Event-Driven in C++

The typed event bus pattern is one of my favourite modern C++ idioms. You get complete
decoupling between publishers and subscribers, and the type system ensures you never publish
to the wrong topic.

Walk through the demo code in `ch2_architectural_styles/src/event_system_demo.cpp`.
Notice how adding a new subscriber requires zero changes to the publisher. That's architectural
decoupling made concrete in code.

---

### Slide 10 — Chapter 3: Requirements

ASRs — Architecturally Significant Requirements — are the ones that drive structural decisions.
"The system should store user data" is not architecturally significant. "The system must handle
10,000 concurrent users with p99 latency under 100ms" very much is, because it drives your
choice of concurrency model, caching strategy, and infrastructure.

The 4+1 view model is useful because it forces you to think about the system from multiple
perspectives. The logical view is what developers think about. The physical view is what ops
thinks about. Architecture has to satisfy both.

---

### Slide 11 — Chapter 4: Fallacies

These eight fallacies are from 1994 and they're still completely relevant. Every single one
of them will bite you in production.

The one I see cause the most damage in practice is number one: "the network is reliable."
Engineers build a beautiful service that calls five other services synchronously. Everything
works great in staging. In production, one of those services has a brief network hiccup.
Now the calling service is slow, which makes its callers slow, which cascades. Within minutes
you have a full outage.

The fix is design for failure from day one. Circuit breakers, timeouts, retries with backoff,
graceful degradation.

---

### Slide 12 — CAP Theorem

CAP is often misunderstood. The key insight is that partition tolerance is not optional in a
real distributed system — networks partition. So the real choice is: when a partition happens,
do you want the system to be consistent (return errors rather than stale data) or available
(return possibly stale data rather than errors)?

Different parts of your system may make different choices. Your financial transactions are
probably CP. Your user preferences cache is probably AP.

---

### Slide 13 — CQRS

CQRS — Command Query Responsibility Segregation — separates the write model from the read
model. At first this sounds unnecessarily complex. But think about what happens when you have
a complex domain with heavy writes *and* complex reporting queries.

If you use one model for both, your query code ends up doing expensive joins that interfere
with write performance, or your write model gets polluted with query optimisation hacks.

CQRS lets you optimise each side independently. Look at `ch4_architectural_system_design/src/cqrs_demo.cpp`
to see this pattern implemented cleanly.

---

### Slide 14 — Chapter 5: C++ Language Features

C++20 gave us a set of features that directly support architectural goals. Let me go through
the most architecturally relevant ones.

`std::optional<T>` is about making nullability explicit. Instead of "this function returns
a pointer that might be null, check the docs" you have "this function returns
`optional<Result>` — the type tells you it might not have a value."

---

### Slide 15 — Strong Types

This is one of those things where the improvement seems trivial until you've seen a real bug
caused by passing arguments in the wrong order.

```cpp
create_account(42, 17);  // is 42 the user_id or account_id?
```

With strong types, the compiler catches this. Zero runtime overhead if the wrapper is trivial.

---

### Slide 16 — Chapter 6: Design Patterns

I want to focus on three patterns that are genuinely different in modern C++ compared to
classic GoF descriptions.

The Observer pattern in classic GoF involves virtual dispatch. In C++, you can use
`std::function` to hold any callable — lambda, functor, member function. The result is
much more flexible.

The Strategy pattern classically involves a virtual base class. In C++, you can use a
template parameter — this is the policy-based design idiom. Zero virtual dispatch overhead.
Used extensively in the STL.

---

### Slide 17 — CRTP

CRTP is one of those patterns that looks strange at first but becomes completely natural
once you understand it. It's static polymorphism — the compiler resolves the dispatch at
compile time rather than runtime.

When would you use this? In performance-critical code where the overhead of a virtual call
matters. Tight inner loops. Real-time systems. Embedded systems.

Look at `ch6_design_patterns_cpp/src/crtp_demo.cpp`. Notice that there is no vtable —
the compiler inlines everything.

---

### Slide 18 — Chapter 7: Build Systems

CMake is the lingua franca of C++ build systems. The modern idiom — target-based with
`target_link_libraries` and `target_compile_features` — is much cleaner than the old
global variable style.

The key insight is that targets are nodes in a dependency graph, and CMake propagates
properties transitively. When you link against a library with `PUBLIC` properties, you
inherit those properties automatically.

---

### Slide 19 — Static Analysis

Static analysis is non-negotiable in production C++ code. The question is not "should we
use it" but "which tool catches which class of bugs."

`AddressSanitizer` catches buffer overflows that would otherwise be silent corruptions in
production. `ThreadSanitizer` catches data races. These are not hypothetical — they find
real bugs in mature C++ codebases every time they're first applied.

Run sanitisers in your CI pipeline, not just locally.

---

### Slide 20 — Chapter 8: Testable Code

The single biggest driver of testable code is keeping your dependencies injectable. If a
class instantiates its dependencies with `new` inside the constructor, you cannot test it
in isolation.

If a class receives its dependencies through the constructor, you can inject fakes in tests
and the real implementations in production.

This is not a testing concern — it's a design concern that has a beneficial side effect on
testability.

---

### Slide 21 — Dependency Injection

Look at `ch8_writing_testable_code/src/testing_demo.cpp`. We have an `order_service` that
depends on a `payment_gateway` and an `inventory` repository. In tests, we inject fakes.
In production, we inject the real implementations.

The key: no `new`, no `std::make_unique<ConcreteImpl>()` inside the service itself.
The service only knows about the interface.

---

### Slide 22 — Chapter 9: CI/CD

CI/CD is infrastructure, not architecture. But it's the infrastructure that makes everything
else possible.

The one principle I'd emphasise: build once, deploy the same artefact everywhere. Do not
build in staging, build in production, build in dev. Build once, in CI, push to a registry,
deploy that exact binary everywhere.

This eliminates an entire class of "but it worked in staging" bugs.

---

### Slide 23 — Chapter 10: Security

Security architecture is about reducing attack surface and making the blast radius of any
breach as small as possible.

The OWASP Top 10 is the minimal reading list for any engineer building systems that process
external input. Number three — injection — is directly relevant to C++ code that parses
network input. Validate everything at the boundary. Never trust external data.

---

### Slide 24 — Secure C++ Idioms

`std::span` is the safe replacement for the classic `void* data, size_t len` pattern that
has caused so many buffer overflows. The size is part of the type — you cannot accidentally
pass a span of the wrong length.

For arithmetic that might overflow — particularly when processing user-supplied sizes —
check for overflow explicitly before performing the operation.

Look at `ch10_security/src/security_demo.cpp` for concrete examples.

---

### Slide 25 — Chapter 11: Performance

The most important rule of performance optimisation: measure first.

I've seen engineers spend days optimising a function that showed up as 0.1% of runtime in
the profile. Meanwhile, 60% of time was spent in an unoptimised SQL query they never looked
at. Profile first, then optimise the actual hotspot.

---

### Slide 26 — Cache-Friendly Design

Modern CPUs are fast. Memory is slow. The gap between L1 cache and main memory is roughly
200× in latency. Cache-friendly code is often the single biggest performance lever in
compute-intensive C++.

The SoA vs AoS demo in `ch11_performance/src/perf_demo.cpp` shows this concretely. When
you update only positions in a physics loop, SoA keeps only positions in cache — not the
mass and health fields you don't need. On 100,000 particles, the difference is measurable.

---

### Slide 27 — Chapter 12: SOA

SOA is about contracts and independence. Before we talk about microservices, it's worth
understanding the simpler form: services with well-defined interfaces that can be deployed
independently.

The protocol choice is important. REST is ubiquitous but verbose. gRPC is fast but requires
schema management. MQTT is perfect for IoT but unusual for service-to-service calls.

There's no universally right answer — it depends on your latency requirements, team
familiarity, and client ecosystem.

---

### Slide 28 — REST vs gRPC

The key difference is not just performance — it's type safety.

With REST + JSON, you can accidentally send `{"amount": "99.99"}` as a string when the
server expects a number. This will either crash at runtime or silently produce wrong results.
With gRPC + protobuf, the schema is shared between client and server. Mismatches are caught
at compile time.

For internal service-to-service calls, gRPC is usually the better choice for this reason.
For public APIs where you don't control the client, REST is often more practical.

---

### Slide 29 — Chapter 13: Microservices

The hardest part of microservices is decomposition. Get it wrong and you have a distributed
monolith — the worst of both worlds. All the operational complexity of microservices, none
of the independence benefits.

The right decomposition follows domain boundaries (DDD bounded contexts), not technical
layers. Don't create a "database service" or a "UI service". Create an "order service",
a "payment service", an "inventory service".

---

### Slide 30 — Saga Pattern

Distributed transactions are one of the hardest problems in microservices. You cannot use
database transactions across service boundaries.

The Saga pattern replaces a single atomic transaction with a sequence of local transactions,
each of which has a compensating transaction that undoes its effects if a later step fails.

The key discipline: design the compensating transactions when you design the forward
transactions. Don't add them as an afterthought.

Look at `ch13_designing_microservices/src/microservice_demo.cpp` — when a payment fails,
the saga orchestrator publishes the cancellation event. No global coordinator needed.

---

### Slide 31 — Observability

In a monolith, when something goes wrong you look at one set of logs. In a microservices
system, a single user request might touch 10 services. How do you debug that?

The answer is distributed tracing with correlation IDs. Every request gets a unique ID at
the edge. Every service propagates that ID in its logs and in downstream calls. Now you can
reconstruct the full trace of a request across all services.

OpenTelemetry is the standard. Instrument it from day one — retrofitting is painful.

---

### Slide 32 — Chapter 14: Containers

Multi-stage Docker builds are the key pattern for C++ container images. Your build stage
needs compilers, CMake, development headers — easily 2GB. Your runtime stage needs none
of that — just the compiled binary and its runtime dependencies.

Always run as a non-root user. Even if your application has no security bugs, running as
root means a successful exploit immediately gets root on the host.

---

### Slide 33 — Kubernetes Essentials

Three things every service needs before going to production in Kubernetes:

One: resource requests and limits. Without them, your pod will be scheduled on an already-
overloaded node, and the kubelet may OOM-kill it. Requests are for scheduling; limits are
for enforcement.

Two: readiness probe. Without one, Kubernetes may route traffic to your pod before it's
ready, causing request failures during startup.

Three: liveness probe. Without one, a deadlocked pod will never be restarted.

---

### Slide 34 — Chapter 15: Cloud-Native

The Twelve-Factor App methodology is 10+ years old and still almost completely relevant.
The most important factor for C++ services is number six: stateless processes.

If your service stores state in memory — sessions, cached data, in-flight requests that
survive restarts — you cannot scale horizontally, and you cannot kill and restart instances
freely. Everything your service needs to survive across restarts must live in external
backing services: databases, caches, message queues.

---

### Slide 35 — Resilience Patterns

The circuit breaker is the most important resilience pattern for services with downstream
dependencies.

Without it: downstream service is slow → your requests queue up → your thread pool
exhausts → your service becomes slow → upstream services queue up → cascade failure.

With it: downstream service is slow → circuit opens after threshold → you fail fast →
return cached result or error → your thread pool stays healthy → no cascade.

---

### Slide 36 — Service Mesh

A service mesh sounds complex — and it is, operationally. But the benefit is that you
stop solving cross-cutting problems in application code.

mTLS between every service. Retries with exponential backoff. Circuit breaking.
Distributed tracing header injection. All of this moves out of your C++ code and into
the sidecar proxy.

The trade-off: your application is simpler, but you now need to understand the mesh
control plane. For large teams with many services, this trade-off is usually worth it.

---

### Slide 37 — Summary

Let me close with the most important meta-principle: architecture is about trade-offs.

There is no "best" architectural style. Microservices are not better than monoliths.
gRPC is not better than REST. SoA layout is not better than AoS layout.

Everything is a trade-off between competing properties — simplicity, performance,
scalability, maintainability, operational complexity, team size, delivery speed.

Your job as an architect is to understand the trade-offs, make an explicit choice,
document why you made it, and be ready to revisit it when the context changes.

---

### Slide 38 — Next Steps

The best way to consolidate this is to apply it. Take a real system you're working on
and write Architecture Decision Records for the three or four most significant structural
decisions. For each one: what were the alternatives? What were the trade-offs? Why did
you choose what you chose?

The act of writing it down forces clarity. Clarity exposes gaps in reasoning. Filling
those gaps makes you a better architect.

Thank you.
