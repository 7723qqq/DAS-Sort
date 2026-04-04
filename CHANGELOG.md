# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0.0] - 2026-04-04

### Added
- **DAS v2**: Quaternary partition with sampled quantiles
  - Sample 32 elements to estimate 1/4, 2/4, 3/4 quantiles
  - Four-way partition: `< p1 | [p1, p2) | [p2, p3) | >= p3`
  - More balanced partitions across different data distributions
- **Extended benchmark**: 14 test cases covering various data patterns
- **Unit tests**: C++ (Catch2) and Python (pytest) test suites
- **Examples**: Log timestamp sorting, sensor time series sorting
- **Benchmark script**: Reproducible performance testing

### Performance (v2 vs std::sort, 100K elements)
| Scenario | DAS v2 | std::sort | Speedup |
|----------|--------|-----------|---------|
| Sorted | 0.04 ms | 0.40 ms | **9x** |
| Reverse | 0.13 ms | 0.52 ms | **4x** |
| AlmostSorted | 0.29 ms | 0.41 ms | **1.4x** |
| AllSame | 0.05 ms | 0.21 ms | **4x** |

## [1.0.0] - 2026-04-03

### Added
- **DAS v1**: Binary partition with density-adaptive pivot selection
  - Pivot: `(min + max) / 2`
  - `is_sorted_check()` for O(n) sorted data detection
  - Insertion sort for small arrays (threshold: 16)
- Core algorithm ~100 lines of code
- Initial performance benchmarks

### Performance (v1 vs std::sort, 10M elements)
| Scenario | DAS v1 | std::sort | Speedup |
|----------|--------|-----------|---------|
| Sorted | 0.006s | 0.070s | **11x** |
| NearlySorted | 0.007s | 0.074s | **10x** |
| ReverseSorted | 0.022s | 0.080s | **3.6x** |

## Version Comparison

| Feature | v1 | v2 |
|---------|----|----|
| Partition | Two-way | Four-way |
| Pivot Selection | `(min+max)/2` | Sampled quantiles |
| Best Case | O(n) | O(n) |
| Average Case | O(n log n) | O(n log n) |
| Sorted Data | 11x faster | 9x faster |
| Random Data | 1.1x slower | 1.35x slower |
