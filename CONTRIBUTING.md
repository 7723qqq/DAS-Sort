# Contributing to DAS-Sort

Thank you for your interest in contributing to DAS-Sort! This document provides guidelines and instructions for contributing.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [How to Contribute](#how-to-contribute)
- [Pull Request Process](#pull-request-process)
- [Coding Standards](#coding-standards)

## Code of Conduct

Be respectful and inclusive. We welcome contributions from everyone.

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/DAS-Sort.git`
3. Create a branch: `git checkout -b feature/your-feature-name`

## Development Setup

### C++ Development

```bash
# Build with MSVC
cl /O2 /EHsc benchmark_extended.cpp

# Build with GCC
g++ -O2 -std=c++17 benchmark_extended.cpp -o benchmark

# Run tests (requires Catch2)
# Download Catch2 from https://github.com/catchorg/Catch2
cl /O2 /EHsc test/test_das.cpp
```

### Python Development

```bash
# Install dependencies
pip install pytest

# Run tests
pytest test/test_das.py -v

# Run benchmark
python benchmark/run_benchmark.py --size 100000
```

## How to Contribute

### Reporting Bugs

1. Check if the bug has already been reported in [Issues](https://github.com/7723qqq/DAS-Sort/issues)
2. If not, create a new issue with:
   - Clear description of the bug
   - Steps to reproduce
   - Expected vs actual behavior
   - Environment (OS, compiler version, etc.)

### Suggesting Enhancements

1. Open an issue with the `enhancement` label
2. Describe the enhancement and its benefits
3. Provide examples if possible

### Submitting Code

1. Make your changes in a feature branch
2. Add tests for new functionality
3. Ensure all tests pass
4. Submit a Pull Request

## Pull Request Process

1. Update documentation if needed
2. Add tests for new features
3. Ensure CI passes (if configured)
4. Request review from maintainers
5. Address review feedback

## Coding Standards

### C++

- Use `snake_case` for functions and variables
- Use `PascalCase` for classes
- Add comments for complex logic
- Follow RAII principles
- Keep functions focused and small

### Python

- Follow PEP 8 style guide
- Use type hints where appropriate
- Write docstrings for public functions
- Keep functions focused and small

### General

- Write clear commit messages
- Keep commits atomic (one feature/fix per commit)
- Test edge cases (empty arrays, single element, etc.)

## Project Structure

```
DAS-Sort/
├── das_v1.hpp          # C++ v1 implementation
├── das_v2.hpp          # C++ v2 implementation
├── das_v1.py           # Python v1 implementation
├── das_v2.py           # Python v2 implementation
├── benchmark/
│   └── run_benchmark.py
├── test/
│   ├── test_das.cpp
│   └── test_das.py
├── examples/
│   ├── log_timestamp_sort.py
│   └── sensor_timeseries_sort.py
├── CHANGELOG.md
├── CONTRIBUTING.md
└── README.md
```

## Questions?

Feel free to open an issue for any questions or discussions.
