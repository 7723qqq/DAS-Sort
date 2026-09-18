/**
 * industrial_bench.cpp - DAS vs industrial-grade sorting implementations
 *
 * Compared (all on std::vector<double>):
 *   std::sort            - libstdc++ introsort
 *   std::stable_sort     - libstdc++ adaptive mergesort (always O(n log n))
 *   pdqsort              - pattern-defeating quicksort (orlp/pdqsort),
 *                          branchless, best-case O(n), worst-case O(n log n)
 *   gfx::timsort         - Timsort port (gfx/cpp-TimSort), the algorithm
 *                          behind Python's sorted() and Java's object sort
 *   DASv2                - this repo, four-way sampled-quantile quicksort
 *   DASv6                - this repo, run-adaptive hybrid
 *
 * Third-party headers are OPTIONAL. If present they are auto-detected:
 *   pdqsort.h        from https://github.com/orlp/pdqsort (Boost license)
 *   gfx/timsort.hpp  from https://github.com/gfx/cpp-TimSort (Boost license)
 *
 * Reproduce:
 *   git clone --depth 1 https://github.com/orlp/pdqsort /tmp/pdqsort
 *   git clone --depth 1 https://github.com/gfx/cpp-TimSort /tmp/timsort
 *   g++ -O2 -std=c++17 -I/tmp/pdqsort -I/tmp/timsort/include \
 *       benchmark/industrial_bench.cpp -o industrial_bench && ./industrial_bench
 *
 * Without the third-party headers the benchmark still runs with the
 * always-available sorters (std::sort / std::stable_sort / DASv2 / DASv6).
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>
#include <string>
#include <functional>

#include "../v2/das_v2.hpp"
#include "../das_v6.hpp"

#if __has_include(<pdqsort.h>)
    #include <pdqsort.h>
    #define HAVE_PDQSORT 1
#else
    #define HAVE_PDQSORT 0
#endif

#if __has_include(<gfx/timsort.hpp>)
    #include <gfx/timsort.hpp>
    #define HAVE_TIMSORT 1
#else
    #define HAVE_TIMSORT 0
#endif

static const int RUNS = 7;  // truncated mean: drop fastest + slowest quartiles

// ============ Scenario generators ============

struct Scenario {
    const char* name;
    std::function<void(std::vector<double>&, std::mt19937&)> gen;
};

static std::vector<Scenario> makeScenarios() {
    return {
        {"Random", [](std::vector<double>& a, std::mt19937& rng) {
            std::uniform_real_distribution<double> d(0.0, 1e7);
            for (auto& x : a) x = d(rng);
        }},
        {"Sorted", [](std::vector<double>& a, std::mt19937&) {
            for (int i = 0; i < (int)a.size(); ++i) a[i] = i;
        }},
        {"Reverse", [](std::vector<double>& a, std::mt19937&) {
            for (int i = 0; i < (int)a.size(); ++i) a[i] = a.size() - i;
        }},
        {"AlmostSort", [](std::vector<double>& a, std::mt19937& rng) {
            for (int i = 0; i < (int)a.size(); ++i) a[i] = i;
            std::uniform_int_distribution<int> p(0, (int)a.size() - 1);
            for (int i = 0; i < std::max(1, (int)a.size() / 1000); ++i)
                std::swap(a[p(rng)], a[p(rng)]);
        }},
        {"OrganPipe", [](std::vector<double>& a, std::mt19937&) {
            int n = (int)a.size();
            for (int i = 0; i < n / 2; ++i) a[i] = i;
            for (int i = n / 2; i < n; ++i) a[i] = n - i;
        }},
        {"Sawtooth", [](std::vector<double>& a, std::mt19937&) {
            int period = std::max(2, (int)a.size() / 10);
            for (int i = 0; i < (int)a.size(); ++i) a[i] = i % period;
        }},
        {"Duplicates", [](std::vector<double>& a, std::mt19937& rng) {
            std::uniform_int_distribution<int> d(0, 99);
            for (auto& x : a) x = d(rng);
        }},
        {"AllSame", [](std::vector<double>& a, std::mt19937&) {
            for (auto& x : a) x = 42.0;
        }},
        {"DescBlocks", [](std::vector<double>& a, std::mt19937&) {
            // n/50 strictly descending blocks (values descend within each
            // block, block ranges ascend): after run-reversal this leaves
            // n/50 ascending runs -> forces DASv6 onto the quicksort fallback
            int runlen = std::max(4, (int)a.size() / 50);
            for (int s = 0; s * runlen < (int)a.size(); ++s) {
                int start = s * runlen;
                int end = std::min((int)a.size(), start + runlen);
                for (int i = start; i < end; ++i) a[i] = start + end - 1 - i;
            }
        }},
        {"ShufWindows", [](std::vector<double>& a, std::mt19937& rng) {
            // 10 shuffled windows of n/100 inside a sorted array
            for (int i = 0; i < (int)a.size(); ++i) a[i] = i;
            int w = std::max(10, (int)a.size() / 100);
            std::uniform_int_distribution<int> p(0, w - 1);
            for (int s = 0; s + w <= (int)a.size(); s += w) {
                for (int i = w - 1; i > 0; --i)
                    std::swap(a[s + i], a[s + p(rng) % (i + 1)]);
            }
        }},
        {"Gaussian", [](std::vector<double>& a, std::mt19937& rng) {
            std::normal_distribution<double> d(0.0, 1.0);
            for (auto& x : a) x = d(rng);
        }},
    };
}

// ============ Sorters under test ============

struct Sorter {
    const char* name;
    std::function<void(std::vector<double>&)> fn;
    bool available;
};

static std::vector<Sorter> makeSorters() {
    std::vector<Sorter> s;
    s.push_back({"std::sort", [](std::vector<double>& a) {
        std::sort(a.begin(), a.end());
    }, true});
    s.push_back({"std::stable", [](std::vector<double>& a) {
        std::stable_sort(a.begin(), a.end());
    }, true});
#if HAVE_PDQSORT
    s.push_back({"pdqsort", [](std::vector<double>& a) {
        pdqsort_branchless(a.begin(), a.end());
    }, true});
#endif
#if HAVE_TIMSORT
    s.push_back({"timsort", [](std::vector<double>& a) {
        gfx::timsort(a.begin(), a.end());
    }, true});
#endif
    s.push_back({"DASv2", [](std::vector<double>& a) {
        DASv2 x; x.sort(a);
    }, true});
    s.push_back({"DASv6", [](std::vector<double>& a) {
        DASv6 x; x.sort(a);
    }, true});
    return s;
}

// ============ Measurement ============

double measure(const std::vector<double>& original,
               const std::function<void(std::vector<double>&)>& fn) {
    std::vector<double> times;
    times.reserve(RUNS);
    for (int r = 0; r < RUNS; ++r) {
        std::vector<double> copy = original;
        auto start = std::chrono::high_resolution_clock::now();
        fn(copy);
        auto end = std::chrono::high_resolution_clock::now();
        times.push_back(std::chrono::duration<double>(end - start).count());
    }
    std::sort(times.begin(), times.end());
    int trim = RUNS / 4;
    double sum = 0.0;
    int count = 0;
    for (int i = trim; i < RUNS - trim; ++i) {
        sum += times[i];
        ++count;
    }
    return (sum / count) * 1000.0;  // ms
}

int main() {
    std::mt19937 rng(42);
    auto scenarios = makeScenarios();
    auto sorters = makeSorters();

    std::cout << "Industrial comparison: ";
    bool first = true;
    for (auto& s : sorters) {
        if (!first) std::cout << " | ";
        std::cout << s.name << (s.available ? "" : " (SKIP)");
        first = false;
    }
    std::cout << "\n";

    for (int n : {100000, 1000000}) {
        std::cout << "\n===== n = " << n << " =====\n";
        std::cout << std::left << std::setw(13) << "Scenario";
        for (auto& s : sorters)
            std::cout << std::left << std::setw(11) << s.name;
        std::cout << "\n" << std::string(13 + 11 * (int)sorters.size(), '-') << "\n";

        for (auto& sc : scenarios) {
            std::vector<double> data(n);
            sc.gen(data, rng);

            // correctness oracle (first run of each scenario)
            std::vector<double> oracle = data;
            std::sort(oracle.begin(), oracle.end());

            std::cout << std::left << std::setw(13) << sc.name;
            for (auto& s : sorters) {
                if (!s.available) {
                    std::cout << std::left << std::setw(11) << "-";
                    continue;
                }
                std::vector<double> check = data;
                s.fn(check);
                if (check != oracle) {
                    std::cout << std::left << std::setw(11) << "WRONG!";
                    continue;
                }
                double ms = measure(data, s.fn);
                std::cout << std::left << std::fixed << std::setprecision(2)
                          << std::setw(11) << ms;
            }
            std::cout << "\n";
        }
    }

    std::cout << "\nNote: each cell is the truncated mean of " << RUNS
              << " runs (ms). DAS implementations carry per-comparison\n";
    std::cout << "instrumentation counters; industrial sorters do not.\n";
    return 0;
}
