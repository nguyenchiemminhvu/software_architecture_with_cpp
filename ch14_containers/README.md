# Chapter 14: Containers

**Book Pages**: 422–455 | *Software Architecture with C++* by Ostrowski & Gaczkowski

> **Note**: This chapter is conceptual — no compiled C++ code. Container tools are
> infrastructure-level, not application-level.

---

## 14.1 Why Containers?

Traditional deployment issues:
- "Works on my machine" — environment divergence between dev/staging/prod
- Long provisioning times for new servers
- Difficult dependency management across services
- Resource utilisation: physical machines are either over-provisioned or fighting for resources

Containers solve these by packaging the application **and its dependencies** into a single,
portable, immutable unit.

```
┌─────────────────────────────────────────────────┐
│   Physical / Virtual Machine                    │
│  ┌──────────────────────────────────────────┐   │
│  │   Container Runtime (Docker / containerd)│   │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────┐  │   │
│  │  │ App A    │ │ App B    │ │ App C    │  │   │
│  │  │ +libs    │ │ +libs    │ │ +libs    │  │   │
│  │  └──────────┘ └──────────┘ └──────────┘  │   │
│  │  Shared OS Kernel                        │   │
│  └──────────────────────────────────────────┘   │
└─────────────────────────────────────────────────┘
```

Containers vs VMs:
| | Container | Virtual Machine |
|---|---|---|
| Start time | < 1 s | 30–60 s |
| Size | MB | GB |
| OS | Shared kernel | Full OS per VM |
| Isolation | Process-level (namespaces) | Hardware-level (hypervisor) |
| Security surface | Larger (shared kernel) | Smaller |

---

## 14.2 Docker

### Core Concepts

- **Image** — immutable read-only layer stack
- **Container** — running instance of an image (adds writable layer)
- **Dockerfile** — declarative recipe to build an image
- **Registry** — storage for images (Docker Hub, ECR, GCR, ACR)

### Multi-Stage Dockerfile for C++ Services

```dockerfile
# Stage 1: Build
FROM ubuntu:22.04 AS builder
RUN apt-get update && apt-get install -y cmake g++ libssl-dev
WORKDIR /src
COPY . .
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)

# Stage 2: Runtime (minimal image)
FROM ubuntu:22.04 AS runtime
RUN apt-get update && apt-get install -y libssl3 && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=builder /src/build/my_service /app/my_service
RUN useradd -r app && chown app:app /app/my_service
USER app
EXPOSE 8080
ENTRYPOINT ["/app/my_service"]
```

Key practices:
1. **Multi-stage builds** — keep runtime image small; build tools stay in the builder stage
2. **Non-root user** — never run as root inside a container
3. **Minimal base image** — use `scratch` or `distroless` for production
4. **Layer caching** — `COPY` dependency files before source code to cache package installs
5. **No secrets in images** — pass secrets via environment variables or secret managers

### Docker Compose (local development)

```yaml
# docker-compose.yml
version: "3.9"
services:
  order-service:
    build: ./order-service
    environment:
      DATABASE_URL: postgres://db:5432/orders
    ports: ["8080:8080"]
    depends_on: [db, broker]

  payment-service:
    build: ./payment-service
    environment:
      DATABASE_URL: postgres://db:5432/payments
    ports: ["8081:8081"]
    depends_on: [db, broker]

  broker:
    image: rabbitmq:3-management
    ports: ["5672:5672", "15672:15672"]

  db:
    image: postgres:15
    volumes: ["pgdata:/var/lib/postgresql/data"]
    environment:
      POSTGRES_PASSWORD: secret

volumes:
  pgdata:
```

---

## 14.3 Kubernetes (K8s)

Kubernetes is the de-facto standard for **container orchestration**:
- **Scheduling** — place containers on nodes with sufficient resources
- **Self-healing** — restart failed containers, reschedule on failed nodes
- **Scaling** — horizontal pod autoscaling based on CPU/memory/custom metrics
- **Rolling updates** — zero-downtime deploys; automatic rollback on failure
- **Service discovery** — built-in DNS for service names

### Key Resources

```
Cluster
└── Node (physical/virtual machine)
    └── Pod (1+ containers sharing network/storage)
        └── Container (Docker image running)

Deployment → manages Pod replicas (ReplicaSet)
Service → stable DNS + load-balanced IP for Pods
ConfigMap → non-secret config data
Secret → encrypted config data
Ingress → external HTTP/HTTPS routing → Service
PersistentVolumeClaim → durable storage for Pods
```

### Sample Deployment + Service

```yaml
# order-service deployment
apiVersion: apps/v1
kind: Deployment
metadata:
  name: order-service
spec:
  replicas: 3
  selector:
    matchLabels: {app: order-service}
  template:
    metadata:
      labels: {app: order-service}
    spec:
      containers:
      - name: order-service
        image: registry.example.com/order-service:v1.2.3
        ports: [{containerPort: 8080}]
        env:
        - name: DB_PASSWORD
          valueFrom:
            secretKeyRef: {name: db-secret, key: password}
        resources:
          requests: {cpu: "100m", memory: "128Mi"}
          limits:   {cpu: "500m", memory: "512Mi"}
        readinessProbe:
          httpGet: {path: /health, port: 8080}
          initialDelaySeconds: 5
---
apiVersion: v1
kind: Service
metadata:
  name: order-service
spec:
  selector: {app: order-service}
  ports: [{port: 80, targetPort: 8080}]
```

### Health Probes

| Probe | Purpose |
|---|---|
| `livenessProbe` | Is the container still alive? Restart if failing |
| `readinessProbe` | Ready to accept traffic? Remove from LB if failing |
| `startupProbe` | Has the app finished starting? Gate other probes |

---

## 14.4 Container Security

- **Image scanning** (Trivy, Snyk, Grype) — scan for CVEs before deployment
- **Read-only root filesystem** — `securityContext.readOnlyRootFilesystem: true`
- **No privilege escalation** — `allowPrivilegeEscalation: false`
- **Drop all capabilities** — `capabilities.drop: ["ALL"]`, add back only what's needed
- **Network policies** — deny all traffic by default; allow-list required paths
- **Pod Security Standards** — use `baseline` or `restricted` policy at namespace level

---

## Key Takeaways

1. **Containers package application + dependencies** — eliminating environment drift
2. **Multi-stage Dockerfiles** keep runtime images small and secure
3. **Never run as root** in containers
4. **Kubernetes automates** scheduling, healing, scaling, and rolling updates
5. **Resource requests/limits** are required — prevent noisy-neighbour problems
6. **Health probes are mandatory** — they enable zero-downtime deploys and self-healing
7. **Scan images for CVEs** before pushing to production registries
