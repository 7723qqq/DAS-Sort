# DAS (Density Adaptive Sort)

**DAS (Density Adaptive Sort)** is an adaptive quicksort algorithm optimized for sorted/nearly-sorted data.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Python-3.8+-green.svg)](https://www.python.org/)
[![CI](https://github.com/7723qqq/DAS-Sort/actions/workflows/ci.yml/badge.svg)](https://github.com/7723qqq/DAS-Sort/actions/workflows/ci.yml)

## Features

- **Adaptive**: Auto-detects sorted data, O(n) time complexity
- **Simple**: Core algorithm ~100 lines of code
- **Fast**: 10-25x faster than std::sort on sorted data
- **Unstable**: Does not preserve relative order of equal elements

## Performance

**Test Environment**: Windows 11, Visual Studio 2022, /O2 optimization

### DAS v2 vs std::sort (100K elements)

| Scenario | DAS v2 | std::sort | Speedup |
|----------|--------|-----------|--------|
| **Sorted** | 0.04 ms | 0.40 ms | **9x faster** |
| **Reverse** | 0.13 ms | 0.52 ms | **4x faster** |
| **AlmostSorted** | 0.29 ms | 0.41 ms | **1.4x faster** |
| **AllSame** | 0.05 ms | 0.21 ms | **4x faster** |
| Random | 7.10 ms | 5.31 ms | 1.3x slower |

## Version Comparison

| Feature | v1 | v2 |
|---------|----|----|
| Partition | Two-way | Four-way |
| Pivot Selection | `(min+max)/2` | Sampled quantiles |
| Best Case | O(n) | O(n) |
| Average Case | O(n log n) | O(n log n) |
| Sorted Data | 11x faster | 9x faster |
| Random Data | 1.1x slower | 1.35x slower |

> **Experimental**: `das_v5.py` is a Python-only experimental version with
> three-way value-band partitioning, reverse-order detection, and a
> counting-sort path for few-unique data. See [CHANGELOG.md](CHANGELOG.md).
>
> **Experimental**: v6 (`das_v6.py` / `das_v6.hpp`) adds Timsort-style run
> detection on top of the quicksort core: one pass finds ascending runs
> (reversing descending ones); few-run data is finished by natural merging
> in O(n·log R) (stable), everything else falls back to v2-style quicksort.
> Best for run-structured data: concatenated sorted segments, organ pipes,
> sawtooths, reversed inputs. On this repo's extended benchmark (100K
> doubles, g++ -O2) v6 beats `std::sort` 32x on PipeOrgan, 2.8x on Sawtooth
> and 8.6x on Reverse.

## Algorithm

### v1: Two-way Partition

```
Pivot: (min + max) / 2
Partition: <= pivot | > pivot
Adaptive: is_sorted_check() for O(n) detection
Small arrays: Insertion sort (threshold 16)
```

### v2: Four-way Partition (Sampled Quantiles)

```
Sampling: 32 elements -> sort -> take 1/4, 2/4, 3/4 quantiles
Partition: < p1 | [p1, p2) | [p2, p3) | >= p3
Advantage: Adaptive to data distribution, more balanced partitions
```

## Quick Start

### C++ (v2)

```cpp
#include "v2/das_v2.hpp"
#include <vector>

int main() {
    std::vector<double> data = {3.0, 1.0, 4.0, 1.0, 5.0};
    
    DASv2 sorter;
    sorter.sort(data);
    
    return 0;
}
```

### Python (v2)

```python
from v2.das_v2 import DASv2

data = [3.0, 1.0, 4.0, 1.0, 5.0]
sorter = DASv2()
sorter.sort(data)
```

## API

### C++

```cpp
void DASv2::sort(std::vector<double>& data);
int64_t DASv2::getComparisons() const;
int64_t DASv2::getSwaps() const;
```

### Python

```python
class DASv2:
    def sort(self, data: list) -> list
    @property
    def comparisons(self) -> int
    @property
    def swaps(self) -> int
```

## Build

```bash
# C++ unit tests (no external dependencies)
g++ -O2 -std=c++17 -Wall -Wextra test/test_das.cpp -o test_das && ./test_das

# Extended benchmark (14 scenarios: DAS v1 / DAS v2 vs std::sort)
g++ -O2 -std=c++17 benchmark_extended.cpp -o benchmark_extended && ./benchmark_extended

# Standalone v1-style benchmark (9 sizes x 8 distributions)
g++ -O2 -std=c++17 das_sort_pure.cpp -o das_bench && ./das_bench

# MSVC (all of the above)
cl /O2 /EHsc test\test_das.cpp
cl /O2 /EHsc benchmark_extended.cpp

# Python tests
pip install pytest
pytest test/test_das.py -v
```

## Project Structure

```
DAS-Sort/
├── v2/
│   ├── __init__.py
│   ├── das_v2.hpp
│   ├── das_v2.py
│   └── BENCHMARK.md
├── das_v1.hpp
├── das_v1.py
├── das_v5.py              # experimental v5 (Python only)
├── das_v6.hpp             # experimental v6: run-adaptive hybrid (C++)
├── das_v6.py              # experimental v6: run-adaptive hybrid (Python)
├── das_sort_pure.cpp      # standalone v1-style sort + benchmark
├── benchmark_extended.cpp # extended 14-scenario benchmark
├── benchmark/
│   └── run_benchmark.py
├── test/
│   ├── test_das.cpp
│   └── test_das.py
├── examples/
│   ├── log_timestamp_sort.py
│   └── sensor_timeseries_sort.py
├── .github/workflows/ci.yml
├── CHANGELOG.md
├── CONTRIBUTING.md
└── README.md
```

## Use Cases

| Scenario | Recommended |
|----------|-------------|
| Sorted/nearly-sorted data | DAS |
| High performance, stability not required | DAS |
| Many duplicate values | DAS |
| Log timestamp sorting | DAS |
| Sensor time series | DAS |
| Run-structured data (concatenated segments, bursts) | DAS v6 (Python) |
| Stable sort needed | std::stable_sort |
| Pure random data | std::sort |

## License

MIT License

## Acknowledgments

- Algorithm design: 7723qqq
- Performance testing: AI Assistant
- Documentation: AI Assistant
