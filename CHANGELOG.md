# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed
- Import paths: tests, examples and the benchmark script now import `DASv2`
  from `v2.das_v2` (matching the README Quick Start); added `v2/__init__.py`
- `test/test_das.cpp` now compiles standalone with a built-in minimal test
  harness (previously required Catch2 and included a non-existent header)
- `test/test_das.cpp` includes fixed to `../v2/das_v2.hpp`

### Added
- `das_v1.hpp`: C++ v1 implementation (was referenced by docs and tests but
  missing from the repository)
- `benchmark_extended.cpp`: extended 14-scenario benchmark comparing DAS v1 /
  DAS v2 against `std::sort` (was referenced by docs but missing)
- `das_v6.py`: run-adaptive hybrid (experimental) - single-pass run
  detection (descending runs reversed in place), natural merge for
  few-run data (O(n·log R), stable) with v2-style quicksort fallback;
  includes `TestDASv6` with a merge-path stability test, a v6 column in
  the benchmark script, and a v6 demo entry point
- `das_v6.hpp`: C++ port of v6 with the same DASv2-style API, reused
  merge buffer, and an explicit-stack quicksort fallback; DASv6 cases in
  `test/test_das.cpp` (merge-path, fallback, cross-version consistency)
  and a DAS v6 column in `benchmark_extended.cpp`
- `TestDASv5` unit tests and cross-consistency tests covering v1/v2/v5/v6
- GitHub Actions CI: Python pytest matrix (3.8 / 3.13) + C++ build & test
- `.gitignore`

### Changed
- Examples now also benchmark DAS v6 on their near-sorted timestamp data
  (v6 is 3-5x faster than v2 there)

### Fixed
- v5 counting-sort path mis-triggered on continuous float data: the
  value-range heuristic (`range < size/10`) passes for any bounded float
  distribution while unique values are unbounded, silently degrading to a
  dict + built-in sort. It now aborts early and falls back to quicksort
  when unique values exceed `min(1024, size/4)`

### Changed
- Python implementations (v1, v2, v5) converted from recursion to explicit
  stacks; `sys.setrecursionlimit` hacks removed (deep or adversarial inputs
  can no longer overflow the interpreter stack)
- **DAS v5**: removed the silent final verification/repair scan - correctness
  is now guaranteed by the algorithm itself; potential bugs surface instead
  of being masked by an O(n^2) fallback
- `das_v5.py` documented in README and CHANGELOG (previously undocumented)
- README project structure and build instructions updated to match the actual
  repository contents

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
