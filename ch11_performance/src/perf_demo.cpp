// perf_demo.cpp
//
// Demonstrates performance-oriented C++ patterns:
//   - AoS vs SoA data layout comparison
//   - String building: += vs reserve + +=
//   - Cache-friendly traversal
//   - Simple timing without external benchmarking library
//
// Book reference: Chapter 11, §11.3-11.4

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

// ============================================================
// TIMER HELPER
// ============================================================

class timer {
    using clock = std::chrono::steady_clock;
    clock::time_point start_ = clock::now();
public:
    void reset() { start_ = clock::now(); }
    double elapsed_ms() const {
        return std::chrono::duration<double, std::milli>(clock::now() - start_).count();
    }
};

// ============================================================
// 1. AoS vs SoA DATA LAYOUT
// ============================================================

// Array of Structs — all fields interleaved
struct particle_aos {
    float x, y, z;        // position
    float vx, vy, vz;     // velocity (not used in position update)
    float mass;            // not used in position update
    float health;          // not used in position update
};

// Struct of Arrays — hot fields together
struct particle_pool_soa {
    std::vector<float> x, y, z;    // positions — hot for physics
    std::vector<float> vx, vy, vz; // velocities — hot for physics
    std::vector<float> mass;       // cold for physics
    std::vector<float> health;     // cold for physics

    explicit particle_pool_soa(size_t n)
        : x(n), y(n), z(n), vx(n, 0.1f), vy(n, 0.1f), vz(n, 0.1f)
        , mass(n, 1.0f), health(n, 100.0f) {}
};

void update_positions_aos(std::vector<particle_aos>& particles, float dt) {
    for (auto& p : particles) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.z += p.vz * dt;
        // Loads mass and health into cache even though not needed
    }
}

void update_positions_soa(particle_pool_soa& pool, float dt) {
    size_t n = pool.x.size();
    for (size_t i = 0; i < n; ++i) {
        pool.x[i] += pool.vx[i] * dt;
        pool.y[i] += pool.vy[i] * dt;
        pool.z[i] += pool.vz[i] * dt;
        // Only x, y, z, vx, vy, vz in cache — very cache-friendly
    }
}

void demo_layout() {
    std::cout << "--- AoS vs SoA Layout ---\n";
    const size_t N = 100'000;
    const float dt = 0.016f; // 60 fps
    const int iterations = 100;

    std::vector<particle_aos> aos_particles(N);
    for (auto& p : aos_particles) { p.x = p.y = p.z = 0; p.vx = p.vy = p.vz = 0.1f; }

    particle_pool_soa soa_pool(N);

    timer t;
    t.reset();
    for (int i = 0; i < iterations; ++i)
        update_positions_aos(aos_particles, dt);
    double aos_ms = t.elapsed_ms();

    t.reset();
    for (int i = 0; i < iterations; ++i)
        update_positions_soa(soa_pool, dt);
    double soa_ms = t.elapsed_ms();

    std::cout << "  AoS: " << aos_ms << " ms ("
              << N << " particles, " << iterations << " iterations)\n";
    std::cout << "  SoA: " << soa_ms << " ms\n";
    if (soa_ms < aos_ms) {
        std::cout << "  SoA is " << (aos_ms / soa_ms) << "x faster\n";
    }
}

// ============================================================
// 2. STRING BUILDING — reserve vs naked +=
// ============================================================

void demo_string_building() {
    std::cout << "\n--- String Building: += vs reserve + += ---\n";
    const int N = 10'000;

    timer t;
    t.reset();
    for (int trial = 0; trial < 100; ++trial) {
        std::string result;
        for (int i = 0; i < N; ++i) result += 'x'; // multiple reallocations
    }
    double naive_ms = t.elapsed_ms();

    t.reset();
    for (int trial = 0; trial < 100; ++trial) {
        std::string result;
        result.reserve(N); // single allocation
        for (int i = 0; i < N; ++i) result += 'x';
    }
    double reserved_ms = t.elapsed_ms();

    std::cout << "  Without reserve: " << naive_ms << " ms\n";
    std::cout << "  With reserve:    " << reserved_ms << " ms\n";
    if (reserved_ms < naive_ms) {
        std::cout << "  Reserve is " << (naive_ms / reserved_ms) << "x faster\n";
    }
}

// ============================================================
// 3. CACHE-FRIENDLY MATRIX TRAVERSAL
// ============================================================

void demo_matrix_traversal() {
    std::cout << "\n--- Cache-Friendly Matrix Traversal ---\n";
    const int SIZE = 512;
    std::vector<float> matrix(SIZE * SIZE, 1.0f);

    timer t;
    // Row-major traversal (cache-friendly — elements in same row are adjacent)
    t.reset();
    double sum_row = 0.0;
    for (int r = 0; r < SIZE; ++r)
        for (int c = 0; c < SIZE; ++c)
            sum_row += matrix[r * SIZE + c];
    double row_ms = t.elapsed_ms();

    // Column-major traversal (cache-unfriendly — cache line discarded after first element)
    t.reset();
    double sum_col = 0.0;
    for (int c = 0; c < SIZE; ++c)
        for (int r = 0; r < SIZE; ++r)
            sum_col += matrix[r * SIZE + c];
    double col_ms = t.elapsed_ms();

    std::cout << "  Row-major traversal:    " << row_ms << " ms (sum=" << sum_row << ")\n";
    std::cout << "  Column-major traversal: " << col_ms << " ms (sum=" << sum_col << ")\n";
    if (row_ms < col_ms) {
        std::cout << "  Row-major is " << (col_ms / row_ms) << "x faster (cache-friendly)\n";
    }
}

// ============================================================
// 4. MATHEMATICAL OPTIMISATION — avoid sqrt/pow where possible
// ============================================================

void demo_math_optimisation() {
    std::cout << "\n--- Math Optimisation ---\n";
    const int N = 1'000'000;
    std::vector<float> a(N), b(N);
    for (int i = 0; i < N; ++i) { a[i] = static_cast<float>(i); b[i] = static_cast<float>(N - i); }

    timer t;
    // Comparing distances: using sqrt
    t.reset();
    int count_sqrt = 0;
    for (int i = 0; i < N; ++i) {
        float dist = std::sqrt(a[i] * a[i] + b[i] * b[i]);
        if (dist < 1000.0f) ++count_sqrt;
    }
    double sqrt_ms = t.elapsed_ms();

    // Comparing distances: avoiding sqrt (compare squared distances)
    t.reset();
    int count_sq = 0;
    for (int i = 0; i < N; ++i) {
        float dist_sq = a[i] * a[i] + b[i] * b[i];
        if (dist_sq < 1000.0f * 1000.0f) ++count_sq;  // compare squared threshold
    }
    double sq_ms = t.elapsed_ms();

    std::cout << "  With sqrt:    " << sqrt_ms << " ms (count=" << count_sqrt << ")\n";
    std::cout << "  Without sqrt: " << sq_ms << " ms (count=" << count_sq << ")\n";
    if (sq_ms < sqrt_ms) {
        std::cout << "  Avoiding sqrt is " << (sqrt_ms / sq_ms) << "x faster\n";
    }
}

// ============================================================
// MAIN
// ============================================================
int main() {
    std::cout << "=== Performance Demo ===\n\n";
    demo_layout();
    demo_string_building();
    demo_matrix_traversal();
    demo_math_optimisation();
    std::cout << "\nDone. Key principle: measure first, optimise the actual bottleneck.\n";
    return 0;
}
