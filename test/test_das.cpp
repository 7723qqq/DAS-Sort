// test/test_das.cpp - DAS v1 / v2 unit tests (self-contained)
//
// No external test framework is required. A minimal harness provides
// TEST_CASE / REQUIRE-style macros with pass/fail summary and a non-zero
// exit code on failure.
//
// Build & run:
//   g++ -O2 -std=c++17 -Wall -Wextra test/test_das.cpp -o test_das && ./test_das
//   cl /O2 /EHsc test\test_das.cpp

#include <vector>
#include <random>
#include <algorithm>
#include <cstdio>
#include <cstring>

#include "../das_v1.hpp"
#include "../das_v6.hpp"
#include "../v2/das_v2.hpp"

// ============ Minimal test harness ============

static int g_checks = 0;
static int g_failures = 0;
static const char* g_current_case = "";

#define REQUIRE(cond)                                                       \
    do {                                                                    \
        ++g_checks;                                                         \
        if (!(cond)) {                                                      \
            ++g_failures;                                                   \
            std::printf("    FAILED [%s] (line %d): %s\n",                  \
                        g_current_case, __LINE__, #cond);                   \
        }                                                                   \
    } while (0)

typedef void (*TestCaseFn)();

static void run_case(const char* name, TestCaseFn fn) {
    g_current_case = name;
    int before = g_failures;
    fn();
    std::printf("%-55s %s\n", name, (g_failures == before) ? "ok" : "FAILED");
}

// ============ Helpers ============

bool isSorted(const std::vector<double>& data) {
    for (size_t i = 0; i + 1 < data.size(); ++i) {
        if (data[i] > data[i + 1]) return false;
    }
    return true;
}

std::vector<double> generateRandom(int n, unsigned int seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 100000);
    std::vector<double> data(n);
    for (int i = 0; i < n; ++i) {
        data[i] = static_cast<double>(dist(rng));
    }
    return data;
}

// ============ DASv1 tests ============

static void test_v1_empty() {
    std::vector<double> data;
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(data.empty());
}

static void test_v1_single() {
    std::vector<double> data = {42.0};
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(data.size() == 1);
    REQUIRE(data[0] == 42.0);
}

static void test_v1_two_sorted() {
    std::vector<double> data = {1.0, 2.0};
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v1_two_reverse() {
    std::vector<double> data = {2.0, 1.0};
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v1_all_identical() {
    std::vector<double> data(1000, 42.0);
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v1_already_sorted() {
    std::vector<double> data;
    for (int i = 0; i < 1000; ++i) data.push_back(static_cast<double>(i));
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v1_reverse_sorted() {
    std::vector<double> data;
    for (int i = 1000; i > 0; --i) data.push_back(static_cast<double>(i));
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v1_random() {
    auto data = generateRandom(10000);
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v1_exactly_16() {
    auto data = generateRandom(16);
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v1_exactly_32() {
    auto data = generateRandom(32);
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v1_duplicates() {
    std::vector<double> data(1000);
    for (int i = 0; i < 1000; ++i) data[i] = static_cast<double>(i % 10);
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v1_negative_values() {
    std::mt19937 rng(7);
    std::uniform_real_distribution<double> dist(-1e6, 1e6);
    std::vector<double> data(5000);
    for (auto& x : data) x = dist(rng);
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

// ============ DASv2 tests ============

static void test_v2_empty() {
    std::vector<double> data;
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(data.empty());
}

static void test_v2_single() {
    std::vector<double> data = {42.0};
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(data.size() == 1);
    REQUIRE(data[0] == 42.0);
}

static void test_v2_two_sorted() {
    std::vector<double> data = {1.0, 2.0};
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v2_two_reverse() {
    std::vector<double> data = {2.0, 1.0};
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v2_all_identical() {
    std::vector<double> data(1000, 42.0);
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v2_already_sorted() {
    std::vector<double> data;
    for (int i = 0; i < 1000; ++i) data.push_back(static_cast<double>(i));
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v2_reverse_sorted() {
    std::vector<double> data;
    for (int i = 1000; i > 0; --i) data.push_back(static_cast<double>(i));
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v2_random() {
    auto data = generateRandom(10000);
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v2_exactly_16() {
    auto data = generateRandom(16);
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v2_exactly_32() {
    auto data = generateRandom(32);
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v2_duplicates() {
    std::vector<double> data(1000);
    for (int i = 0; i < 1000; ++i) data[i] = static_cast<double>(i % 10);
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v2_nearly_sorted() {
    std::vector<double> data;
    for (int i = 0; i < 10000; ++i) data.push_back(static_cast<double>(i));
    std::mt19937 rng(42);
    for (int i = 0; i < 10; ++i) {
        int a = rng() % 10000;
        int b = rng() % 10000;
        std::swap(data[a], data[b]);
    }
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

// ============ DASv6 tests ============

static void test_v6_empty() {
    std::vector<double> data;
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(data.empty());
}

static void test_v6_single() {
    std::vector<double> data = {42.0};
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(data.size() == 1);
    REQUIRE(data[0] == 42.0);
}

static void test_v6_two_sorted() {
    std::vector<double> data = {1.0, 2.0};
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_two_reverse() {
    std::vector<double> data = {2.0, 1.0};
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_all_identical() {
    std::vector<double> data(1000, 42.0);
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_already_sorted() {
    std::vector<double> data;
    for (int i = 0; i < 1000; ++i) data.push_back(static_cast<double>(i));
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_reverse_sorted() {
    std::vector<double> data;
    for (int i = 1000; i > 0; --i) data.push_back(static_cast<double>(i));
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_random() {
    auto data = generateRandom(10000);
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_exactly_16() {
    auto data = generateRandom(16);
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_exactly_32() {
    auto data = generateRandom(32);
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_duplicates() {
    std::vector<double> data(1000);
    for (int i = 0; i < 1000; ++i) data[i] = static_cast<double>(i % 10);
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_organ_pipe_merge_path() {
    // 2 runs -> natural merge path
    std::vector<double> data;
    for (int i = 0; i < 5000; ++i) data.push_back(static_cast<double>(i));
    for (int i = 5000; i > 0; --i) data.push_back(static_cast<double>(i));
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_concat_segments_merge_path() {
    // 4 internally-sorted shuffled segments -> merge path
    std::vector<double> data;
    const int seg_order[4] = {3, 0, 2, 1};
    for (int s = 0; s < 4; ++s) {
        for (int i = 0; i < 1000; ++i) {
            data.push_back(static_cast<double>(seg_order[s] * 1000 + i));
        }
    }
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_nearly_sorted() {
    std::vector<double> data;
    for (int i = 0; i < 10000; ++i) data.push_back(static_cast<double>(i));
    std::mt19937 rng(42);
    for (int i = 0; i < 10; ++i) {
        int a = rng() % 10000;
        int b = rng() % 10000;
        std::swap(data[a], data[b]);
    }
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

static void test_v6_sawtooth_merge_path() {
    // 10 ascending sawtooth periods -> merge path
    std::vector<double> data;
    for (int i = 0; i < 10000; ++i) {
        data.push_back(static_cast<double>(i % 1000));
    }
    DASv6 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

// ============ Cross-version comparison ============

static void test_compare_v1_v2_v6() {
    for (int trial = 0; trial < 10; ++trial) {
        auto data1 = generateRandom(1000, trial);
        auto data2 = data1;
        auto data6 = data1;

        DASv1 v1;
        DASv2 v2;
        DASv6 v6;
        v1.sort(data1);
        v2.sort(data2);
        v6.sort(data6);

        REQUIRE(isSorted(data1));
        REQUIRE(isSorted(data2));
        REQUIRE(isSorted(data6));
        REQUIRE(data1 == data2);
        REQUIRE(data1 == data6);
    }
}

int main() {
    std::printf("Running DAS unit tests...\n\n");

    run_case("DASv1 - Empty array", test_v1_empty);
    run_case("DASv1 - Single element", test_v1_single);
    run_case("DASv1 - Two elements sorted", test_v1_two_sorted);
    run_case("DASv1 - Two elements reverse", test_v1_two_reverse);
    run_case("DASv1 - All identical", test_v1_all_identical);
    run_case("DASv1 - Already sorted", test_v1_already_sorted);
    run_case("DASv1 - Reverse sorted", test_v1_reverse_sorted);
    run_case("DASv1 - Random data", test_v1_random);
    run_case("DASv1 - Exactly 16 elements (insertion threshold)", test_v1_exactly_16);
    run_case("DASv1 - Exactly 32 elements", test_v1_exactly_32);
    run_case("DASv1 - Duplicates", test_v1_duplicates);
    run_case("DASv1 - Negative values", test_v1_negative_values);

    run_case("DASv2 - Empty array", test_v2_empty);
    run_case("DASv2 - Single element", test_v2_single);
    run_case("DASv2 - Two elements sorted", test_v2_two_sorted);
    run_case("DASv2 - Two elements reverse", test_v2_two_reverse);
    run_case("DASv2 - All identical", test_v2_all_identical);
    run_case("DASv2 - Already sorted", test_v2_already_sorted);
    run_case("DASv2 - Reverse sorted", test_v2_reverse_sorted);
    run_case("DASv2 - Random data", test_v2_random);
    run_case("DASv2 - Exactly 16 elements (insertion threshold)", test_v2_exactly_16);
    run_case("DASv2 - Exactly 32 elements (sample size)", test_v2_exactly_32);
    run_case("DASv2 - Duplicates", test_v2_duplicates);
    run_case("DASv2 - Nearly sorted (0.1% unsorted)", test_v2_nearly_sorted);

    run_case("DASv6 - Empty array", test_v6_empty);
    run_case("DASv6 - Single element", test_v6_single);
    run_case("DASv6 - Two elements sorted", test_v6_two_sorted);
    run_case("DASv6 - Two elements reverse", test_v6_two_reverse);
    run_case("DASv6 - All identical", test_v6_all_identical);
    run_case("DASv6 - Already sorted", test_v6_already_sorted);
    run_case("DASv6 - Reverse sorted", test_v6_reverse_sorted);
    run_case("DASv6 - Random data", test_v6_random);
    run_case("DASv6 - Exactly 16 elements (insertion threshold)", test_v6_exactly_16);
    run_case("DASv6 - Exactly 32 elements", test_v6_exactly_32);
    run_case("DASv6 - Duplicates", test_v6_duplicates);
    run_case("DASv6 - Organ pipe (merge path)", test_v6_organ_pipe_merge_path);
    run_case("DASv6 - Concatenated segments (merge path)", test_v6_concat_segments_merge_path);
    run_case("DASv6 - Nearly sorted", test_v6_nearly_sorted);
    run_case("DASv6 - Sawtooth (merge path)", test_v6_sawtooth_merge_path);

    run_case("Compare v1 vs v2 vs v6 - Same result", test_compare_v1_v2_v6);

    std::printf("\n%d checks, %d failures\n", g_checks, g_failures);
    if (g_failures == 0) {
        std::printf("ALL TESTS PASSED\n");
        return 0;
    }
    std::printf("TESTS FAILED\n");
    return 1;
}
