#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <vector>
#include <random>
#include <algorithm>

#include "../das_v1.hpp"
#include "../das_v2.hpp"

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

TEST_CASE("DASv1 - Empty array", "[das_v1]") {
    std::vector<double> data;
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(data.empty());
}

TEST_CASE("DASv1 - Single element", "[das_v1]") {
    std::vector<double> data = {42.0};
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(data.size() == 1);
    REQUIRE(data[0] == 42.0);
}

TEST_CASE("DASv1 - Two elements sorted", "[das_v1]") {
    std::vector<double> data = {1.0, 2.0};
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv1 - Two elements reverse", "[das_v1]") {
    std::vector<double> data = {2.0, 1.0};
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv1 - All identical", "[das_v1]") {
    std::vector<double> data(1000, 42.0);
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv1 - Already sorted", "[das_v1]") {
    std::vector<double> data;
    for (int i = 0; i < 1000; ++i) data.push_back(static_cast<double>(i));
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv1 - Reverse sorted", "[das_v1]") {
    std::vector<double> data;
    for (int i = 1000; i > 0; --i) data.push_back(static_cast<double>(i));
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv1 - Random data", "[das_v1]") {
    auto data = generateRandom(10000);
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv1 - Exactly 16 elements (insertion threshold)", "[das_v1]") {
    auto data = generateRandom(16);
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv1 - Exactly 32 elements", "[das_v1]") {
    auto data = generateRandom(32);
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv1 - Duplicates", "[das_v1]") {
    std::vector<double> data(1000);
    for (int i = 0; i < 1000; ++i) {
        data[i] = static_cast<double>(i % 10);
    }
    DASv1 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv2 - Empty array", "[das_v2]") {
    std::vector<double> data;
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(data.empty());
}

TEST_CASE("DASv2 - Single element", "[das_v2]") {
    std::vector<double> data = {42.0};
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(data.size() == 1);
    REQUIRE(data[0] == 42.0);
}

TEST_CASE("DASv2 - Two elements sorted", "[das_v2]") {
    std::vector<double> data = {1.0, 2.0};
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv2 - Two elements reverse", "[das_v2]") {
    std::vector<double> data = {2.0, 1.0};
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv2 - All identical", "[das_v2]") {
    std::vector<double> data(1000, 42.0);
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv2 - Already sorted", "[das_v2]") {
    std::vector<double> data;
    for (int i = 0; i < 1000; ++i) data.push_back(static_cast<double>(i));
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv2 - Reverse sorted", "[das_v2]") {
    std::vector<double> data;
    for (int i = 1000; i > 0; --i) data.push_back(static_cast<double>(i));
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv2 - Random data", "[das_v2]") {
    auto data = generateRandom(10000);
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv2 - Exactly 16 elements (insertion threshold)", "[das_v2]") {
    auto data = generateRandom(16);
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv2 - Exactly 32 elements (sample size)", "[das_v2]") {
    auto data = generateRandom(32);
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv2 - Duplicates", "[das_v2]") {
    std::vector<double> data(1000);
    for (int i = 0; i < 1000; ++i) {
        data[i] = static_cast<double>(i % 10);
    }
    DASv2 sorter;
    sorter.sort(data);
    REQUIRE(isSorted(data));
}

TEST_CASE("DASv2 - Nearly sorted (0.1% unsorted)", "[das_v2]") {
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

TEST_CASE("Compare v1 vs v2 - Same result", "[compare]") {
    for (int trial = 0; trial < 10; ++trial) {
        auto data1 = generateRandom(1000, trial);
        auto data2 = data1;
        
        DASv1 v1;
        DASv2 v2;
        v1.sort(data1);
        v2.sort(data2);
        
        REQUIRE(isSorted(data1));
        REQUIRE(isSorted(data2));
        REQUIRE(data1 == data2);
    }
}
