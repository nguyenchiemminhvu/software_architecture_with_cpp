# Chapter 15: Cloud-Native Design

**Book Pages**: 456–491 | *Software Architecture with C++* by Ostrowski & Gaczkowski

> **Note**: This chapter is conceptual — no compiled C++ code. Cloud-native patterns are
> infrastructure and architecture decisions, not language features.

---

## 15.1 What Does "Cloud-Native" Mean?

Cloud-native is a **design philosophy** — not just "runs in the cloud". The CNCF defines it as:

> Systems that are scalable, resilient, and observable, built as loosely coupled services
> deployed using DevOps practices on dynamic, orchestrated infrastructure.

Key attributes:

| Attribute | What it means |
|---|---|
| **Scalable** | Scale horizontally on demand without re-architecture |
| **Resilient** | Self-heal; no single point of failure |
| **Observable** | Metrics, traces, logs are first-class concerns |
| **Automated** | CI/CD, auto-scaling, auto-recovery |
| **Loosely coupled** | Services change independently |

The [Twelve-Factor App](https://12factor.net/) methodology captures cloud-native best practices.

---

## 15.2 The Twelve-Factor App

| Factor | Principle |
|---|---|
| I. Codebase | One codebase per service; tracked in VCS |
| II. Dependencies | Explicitly declare and isolate dependencies |
| III. Config | Store config in environment (not in code) |
| IV. Backing services | Treat DBs, queues, caches as attached resources |
| V. Build/Release/Run | Strictly separate build, release, run stages |
| VI. Processes | Execute app as stateless processes |
| VII. Port binding | Export services via port binding |
| VIII. Concurrency | Scale out via process model |
| IX. Disposability | Fast startup; graceful shutdown |
| X. Dev/Prod parity | Keep dev, staging, prod as similar as possible |
| XI. Logs | Treat logs as event streams (stdout, not files) |
| XII. Admin processes | Run admin tasks as one-off processes |

```
Factor VI — Stateless Processes:
  State must live in external backing services (DB, cache, queue)
  Processes can be killed and restarted at any time
  Sticky sessions are an anti-pattern

Factor IX — Disposability:
  Target startup time < 5 seconds
  Handle SIGTERM: drain in-flight requests, close DB connections, exit cleanly
  Kubernetes sends SIGTERM 30s before force-killing (terminationGracePeriodSeconds)
```

---

## 15.3 Cloud-Native Patterns

### Circuit Breaker

```
Closed → (failure count exceeds threshold) → Open
Open → (timeout elapsed) → Half-Open → (success) → Closed
                                       → (failure) → Open

When Open: fail fast, return fallback, do not call downstream
```

### Bulkhead

Isolate resources so that one failing component cannot exhaust all system resources:

```
Without bulkhead:
  All service calls share one thread pool
  Slow DB calls exhaust pool → all services unresponsive

With bulkhead:
  order-service DB: pool of 20 threads
  payment-service: pool of 10 threads
  user-service:    pool of 10 threads
  Each service fails independently
```

### Sidecar

Deploy cross-cutting concerns as a separate container in the same Pod:

```
Pod
├── app container (your C++ service)
└── sidecar container (Envoy proxy / log forwarder / secrets injector)
```

Used by service meshes (Istio, Linkerd) to inject mTLS, retries, telemetry transparently.

### Strangler Fig (migration pattern)

Incrementally migrate a monolith to microservices:

```
Phase 1: All traffic → Monolith
Phase 2: New feature → Microservice A + Monolith (via strangler proxy)
Phase 3: Migrate feature X → Microservice B; proxy redirects /feature-x
Phase 4: Monolith shrinks; microservices grow
Phase N: Monolith retired
```

---

## 15.4 Service Mesh

A service mesh provides a dedicated infrastructure layer for service-to-service communication:

```
┌────────────────────────────────────────────────────────────┐
│ Control Plane (Istiod)                                     │
│   Policy, CA, config distribution                          │
└────────────────────┬───────────────────────────────────────┘
                     │ configure
        ┌────────────┴────────────┐
        │                         │
   Pod A                      Pod B
  ┌─────────────┐           ┌─────────────┐
  │ App + Envoy │──mTLS────▶│ App + Envoy │
  └─────────────┘           └─────────────┘

Envoy sidecar provides (transparent to app):
  - mTLS between all services
  - Retries, timeouts, circuit breaking
  - L7 load balancing
  - Distributed tracing (auto-inject span headers)
  - Telemetry (metrics exported to Prometheus)
```

Service mesh capabilities:

| Capability | Description |
|---|---|
| **mTLS** | All east-west traffic encrypted and authenticated |
| **Traffic management** | Canary, A/B, blue-green at the network layer |
| **Retries** | Automatic retry with exponential backoff |
| **Circuit breaking** | Automatic at the proxy layer |
| **Observability** | Auto-generated distributed traces and metrics |
| **Fault injection** | Inject delays and errors for chaos testing |

---

## 15.5 Cloud-Native Storage

Stateless services + stateful backing services:

| Storage Type | Use Case | Examples |
|---|---|---|
| **Object storage** | Blobs, media, backups | S3, GCS, Azure Blob |
| **Relational DB** | Structured, transactional data | Cloud SQL, RDS, Aurora |
| **NoSQL / document** | Flexible schema, high throughput | DynamoDB, Firestore, MongoDB Atlas |
| **Cache** | Low-latency read-through | Elasticache (Redis), Memorystore |
| **Message queue** | Async decoupled communication | SQS, Pub/Sub, MSK (Kafka) |
| **Time-series DB** | Metrics, observability | InfluxDB, Prometheus TSDB |

---

## 15.6 Auto-Scaling

### Horizontal Pod Autoscaler (HPA)

```yaml
apiVersion: autoscaling/v2
kind: HorizontalPodAutoscaler
metadata:
  name: order-service-hpa
spec:
  scaleTargetRef:
    apiVersion: apps/v1
    kind: Deployment
    name: order-service
  minReplicas: 2
  maxReplicas: 20
  metrics:
  - type: Resource
    resource:
      name: cpu
      target:
        type: Utilization
        averageUtilization: 70
```

### KEDA (Kubernetes Event-Driven Autoscaling)

Scale based on queue depth, Kafka lag, or any custom metric — scale to zero when idle.

---

## 15.7 Multi-Region and Disaster Recovery

| Strategy | RPO | RTO | Cost |
|---|---|---|---|
| **Backup & restore** | Hours | Hours | Low |
| **Pilot light** | Minutes | Minutes | Medium |
| **Warm standby** | Seconds | Seconds | High |
| **Active-active multi-region** | Near-zero | Near-zero | Very high |

Key considerations:
- **Data sovereignty**: which regions are allowed to store which data?
- **Latency**: cross-region replication adds write latency
- **Consistency**: strong consistency vs eventual consistency across regions
- **Cost**: data egress fees can dominate at scale

---

## Key Takeaways

1. **Cloud-native is a design philosophy** — scalable, resilient, observable, automated
2. **Twelve-Factor App** — the canonical checklist for cloud-native service design
3. **Stateless services** — state lives in backing services, processes are disposable
4. **Circuit breaker + bulkhead** — prevent cascading failures
5. **Service mesh** — moves mTLS, retries, tracing out of application code
6. **Auto-scaling requires good health probes and resource limits** — set both before
   enabling HPA
7. **Design for failure** — every downstream call will eventually fail; plan the degraded path
