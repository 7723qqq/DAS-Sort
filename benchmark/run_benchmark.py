#!/usr/bin/env python3
"""
DAS Sort Benchmark Script
Generates test data and runs performance comparison

Usage:
    python run_benchmark.py [--size N] [--trials N]
"""

import subprocess
import sys
import os
import time
import random
import argparse
from pathlib import Path


def generate_test_file(filename, data_size, data_type):
    """Generate test data file"""
    random.seed(42)
    
    if data_type == "sorted":
        data = list(range(data_size))
    elif data_type == "reverse":
        data = list(range(data_size, 0, -1))
    elif data_type == "random":
        data = [random.randint(0, 1000000) for _ in range(data_size)]
    elif data_type == "nearly_sorted":
        data = list(range(data_size))
        for _ in range(data_size // 1000):
            a = random.randint(0, data_size - 1)
            b = random.randint(0, data_size - 1)
            data[a], data[b] = data[b], data[a]
    elif data_type == "duplicates":
        data = [random.randint(0, 100) for _ in range(data_size)]
    elif data_type == "all_same":
        data = [42] * data_size
    else:
        data = [random.randint(0, 1000000) for _ in range(data_size)]
    
    with open(filename, 'w') as f:
        f.write(f"{data_size}\n")
        for val in data:
            f.write(f"{val}\n")
    
    return data


def run_cpp_benchmark(executable, data_file):
    """Run C++ benchmark"""
    start = time.perf_counter()
    result = subprocess.run(
        [executable, data_file],
        capture_output=True,
        text=True
    )
    elapsed = time.perf_counter() - start
    return elapsed, result.stdout


def run_python_benchmark(sorter_class, data):
    """Run Python benchmark"""
    data_copy = data.copy()
    sorter = sorter_class()
    start = time.perf_counter()
    sorter.sort(data_copy)
    elapsed = time.perf_counter() - start
    return elapsed


def main():
    parser = argparse.ArgumentParser(description='DAS Sort Benchmark')
    parser.add_argument('--size', type=int, default=100000, help='Data size')
    parser.add_argument('--trials', type=int, default=3, help='Number of trials')
    args = parser.parse_args()
    
    print("=" * 70)
    print(f"DAS Sort Benchmark - Data size: {args.size:,}")
    print("=" * 70)
    
    data_types = ["sorted", "reverse", "random", "nearly_sorted", "duplicates", "all_same"]
    
    sys.path.insert(0, str(Path(__file__).parent.parent))
    from das_v1 import DASv1
    from das_v2 import DASv2
    
    print(f"\n{'Type':<15} | {'Python v1':>12} | {'Python v2':>12} | {'Built-in':>12}")
    print("-" * 60)
    
    for dtype in data_types:
        random.seed(42)
        
        if dtype == "sorted":
            data = list(range(args.size))
        elif dtype == "reverse":
            data = list(range(args.size, 0, -1))
        elif dtype == "random":
            data = [random.randint(0, 1000000) for _ in range(args.size)]
        elif dtype == "nearly_sorted":
            data = list(range(args.size))
            for _ in range(args.size // 1000):
                a = random.randint(0, args.size - 1)
                b = random.randint(0, args.size - 1)
                data[a], data[b] = data[b], data[a]
        elif dtype == "duplicates":
            data = [random.randint(0, 100) for _ in range(args.size)]
        elif dtype == "all_same":
            data = [42] * args.size
        
        v1_times = []
        v2_times = []
        builtin_times = []
        
        for _ in range(args.trials):
            t1 = run_python_benchmark(DASv1, data)
            v1_times.append(t1)
            
            t2 = run_python_benchmark(DASv2, data)
            v2_times.append(t2)
            
            data_copy = data.copy()
            start = time.perf_counter()
            data_copy.sort()
            builtin_times.append(time.perf_counter() - start)
        
        avg_v1 = sum(v1_times) / len(v1_times) * 1000
        avg_v2 = sum(v2_times) / len(v2_times) * 1000
        avg_builtin = sum(builtin_times) / len(builtin_times) * 1000
        
        print(f"{dtype:<15} | {avg_v1:>10.2f} ms | {avg_v2:>10.2f} ms | {avg_builtin:>10.2f} ms")
    
    print("\n" + "=" * 70)
    print("Note: Python built-in sort uses Timsort (adaptive merge sort)")
    print("=" * 70)


if __name__ == "__main__":
    main()
