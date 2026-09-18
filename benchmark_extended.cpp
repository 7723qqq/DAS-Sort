/**
 * benchmark_extended.cpp - Extended benchmark: DAS v1 / v2 / v6 vs std::sort
 *
 * Reproduces the 14 scenarios documented in v2/BENCHMARK.md:
 *   Random, Sorted, Reverse, Duplicates, Normal, Skewed, AlmostSort,
 *   AlmostRev, AllSame, Bimodal, SmallRange, LogNormal, Sawtooth, PipeOrgan
 *
 * v6 additionally exploits run structure (Sorted, Reverse, Sawtooth,
 * PipeOrgan, and other patterns made of a few ascending runs).
 *
 * Build:
 *   g++ -O2 -std=c++17 benchmark_extended.cpp -o benchmark_extended
 *   cl /O2 /EHsc benchmark_extended.cpp
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <string>

#include "das_v1.hpp"
#include "das_v6.hpp"
#include "v2/das_v2.hpp"

static const int DATA_SIZE = 100000;
static const int RUNS = 7;   // truncated mean: drop slowest + fastest quartiles

// ============ Data generators ============

void gen_random(std::vector<double>& arr, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1000000.0);
    for (auto& x : arr) x = dist(rng);
}

void gen_sorted(std::vector<double>& arr, std::mt19937&) {
    for (int i = 0; i < (int)arr.size(); ++i) arr[i] = (double)i;
}

void gen_reverse(std::vector<double>& arr, std::mt19937&) {
    for (int i = 0; i < (int)arr.size(); ++i) arr[i] = (double)(arr.size() - i);
}

void gen_duplicates(std::vector<double>& arr, std::mt19937& rng) {
    std::uniform_int_distribution<int> dist(0, 99);  // 100 unique values
    for (auto& x : arr) x = (double)dist(rng);
}

void gen_normal(std::vector<double>& arr, std::mt19937& rng) {
    std::normal_distribution<double> dist(500000.0, 100000.0);
    for (auto& x : arr) x = dist(rng);
}

void gen_skewed(std::vector<double>& arr, std::mt19937& rng) {
    std::exponential_distribution<double> dist(0.00001);
    for (auto& x : arr) x = dist(rng);
}

void gen_almost_sorted(std::vector<double>& arr, std::mt19937& rng) {
    for (int i = 0; i < (int)arr.size(); ++i) arr[i] = (double)i;
    // 99.9% sorted: swap ~0.1% of positions
    int swaps = std::max(1, (int)arr.size() / 1000);
    std::uniform_int_distribution<int> pos(0, (int)arr.size() - 1);
    for (int i = 0; i < swaps; ++i) {
        std::swap(arr[pos(rng)], arr[pos(rng)]);
    }
}

void gen_almost_reverse(std::vector<double>& arr, std::mt19937& rng) {
    for (int i = 0; i < (int)arr.size(); ++i) arr[i] = (double)(arr.size() - i);
    int swaps = std::max(1, (int)arr.size() / 1000);
    std::uniform_int_distribution<int> pos(0, (int)arr.size() - 1);
    for (int i = 0; i < swaps; ++i) {
        std::swap(arr[pos(rng)], arr[pos(rng)]);
    }
}

void gen_all_same(std::vector<double>& arr, std::mt19937&) {
    for (auto& x : arr) x = 42.0;
}

void gen_bimodal(std::vector<double>& arr, std::mt19937& rng) {
    std::normal_distribution<double> low(250000.0, 20000.0);
    std::normal_distribution<double> high(750000.0, 20000.0);
    std::bernoulli_distribution coin(0.5);
    for (auto& x : arr) x = coin(rng) ? low(rng) : high(rng);
}

void gen_small_range(std::vector<double>& arr, std::mt19937& rng) {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (auto& x : arr) x = dist(rng);
}

void gen_log_normal(std::vector<double>& arr, std::mt19937& rng) {
    std::lognormal_distribution<double> dist(0.0, 1.0);
    for (auto& x : arr) x = dist(rng);
}

void gen_sawtooth(std::vector<double>& arr, std::mt19937&) {
    int period = std::max(2, (int)arr.size() / 10);
    for (int i = 0; i < (int)arr.size(); ++i) arr[i] = (double)(i % period);
}

void gen_pipe_organ(std::vector<double>& arr, std::mt19937&) {
    int n = (int)arr.size();
    for (int i = 0; i < n / 2; ++i) arr[i] = (double)i;
    for (int i = n / 2; i < n; ++i) arr[i] = (double)(n - i);
}

// ============ Measurement ============

bool verify_sorted(const std::vector<double>& arr) {
    for (size_t i = 0; i + 1 < arr.size(); ++i) {
        if (arr[i] > arr[i + 1]) return false;
    }
    return true;
}

using SortFn = void (*)(std::vector<double>&);

static void sort_std(std::vector<double>& a) { std::sort(a.begin(), a.end()); }
static void sort_v1(std::vector<double>& a) { DASv1 s; s.sort(a); }
static void sort_v2(std::vector<double>& a) { DASv2 s; s.sort(a); }
static void sort_v6(std::vector<double>& a) { DASv6 s; s.sort(a); }

// Truncated mean of `runs` timings (drop the fastest and slowest quartiles)
double measure(const std::vector<double>& original, SortFn fn) {
    std::vector<double> times;
    times.reserve(RUNS);
    for (int r = 0; r < RUNS; ++r) {
        std::vector<double> copy = original;
        auto start = std::chrono::high_resolution_clock::now();
        fn(copy);
        auto end = std::chrono::high_resolution_clock::now();
        if (!verify_sorted(copy)) {
            std::cout << "SORT CORRECTNESS ERROR!" << std::endl;
            return -1.0;
        }
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

    std::cout << "=======================================================================\n";
    std::cout << "     DAS Extended Benchmark: DAS v1 / v2 / v6 vs std::sort\n";
    std::cout << "     " << DATA_SIZE << " elements, " << RUNS
              << " runs (truncated mean)\n";
    std::cout << "=======================================================================\n\n";

    std::cout << std::left
              << std::setw(13) << "Scenario"
              << std::setw(11) << "std::sort"
              << std::setw(11) << "DAS v1"
              << std::setw(11) << "DAS v2"
              << std::setw(11) << "DAS v6"
              << std::setw(10) << "Winner"
              << std::setw(10) << "Speedup" << std::endl;
    std::cout << "-----------------------------------------------------------------------\n";

    struct Scenario {
        const char* name;
        void (*gen)(std::vector<double>&, std::mt19937&);
    };
    std::vector<Scenario> scenarios = {
        {"Random", gen_random},
        {"Sorted", gen_sorted},
        {"Reverse", gen_reverse},
        {"Duplicates", gen_duplicates},
        {"Normal", gen_normal},
        {"Skewed", gen_skewed},
        {"AlmostSort", gen_almost_sorted},
        {"AlmostRev", gen_almost_reverse},
        {"AllSame", gen_all_same},
        {"Bimodal", gen_bimodal},
        {"SmallRange", gen_small_range},
        {"LogNormal", gen_log_normal},
        {"Sawtooth", gen_sawtooth},
        {"PipeOrgan", gen_pipe_organ},
    };

    int std_wins = 0, das_wins = 0;
    const char* das_names[3] = {"DAS v1", "DAS v2", "DAS v6"};

    for (const auto& sc : scenarios) {
        std::vector<double> data(DATA_SIZE);
        sc.gen(data, rng);

        double t_std = measure(data, sort_std);
        double t_das[3];
        t_das[0] = measure(data, sort_v1);
        t_das[1] = measure(data, sort_v2);
        t_das[2] = measure(data, sort_v6);

        if (t_std < 0) return 1;
        for (double t : t_das) {
            if (t < 0) return 1;
        }

        int best_das = 0;
        for (int i = 1; i < 3; ++i) {
            if (t_das[i] < t_das[best_das]) best_das = i;
        }

        const char* winner;
        double speedup;
        if (t_std < t_das[best_das]) {
            winner = "std::sort";
            ++std_wins;
            speedup = t_std / t_das[best_das];
        } else {
            winner = das_names[best_das];
            ++das_wins;
            speedup = t_das[best_das] / t_std;
        }

        std::cout << std::left << std::setw(13) << sc.name
                  << std::setw(11) << std::fixed << std::setprecision(2) << t_std
                  << std::setw(11) << t_das[0]
                  << std::setw(11) << t_das[1]
                  << std::setw(11) << t_das[2]
                  << std::setw(10) << winner
                  << std::setprecision(1) << speedup << "x "
                  << ((winner[0] == 's') ? "std" : "DAS")
                  << std::endl;
    }

    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << "Wins: std::sort=" << std_wins << "  DAS=" << das_wins << std::endl;
    std::cout << "\nExpected: DAS typically wins on adaptive patterns such as\n";
    std::cout << "Sorted / Reverse / Sawtooth / PipeOrgan (O(n) detection or run\n";
    std::cout << "merging); std::sort wins on random-like data. Exact margins vary\n";
    std::cout << "by platform and compiler.\n";

    return 0;
}
