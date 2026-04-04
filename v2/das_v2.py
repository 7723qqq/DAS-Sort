#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
DAS Sort v2.0 - Quaternary partition with sampled quantiles
Author: Original
Date: 2026

Core improvements:
- Sample 32 elements, sort them, take 1/4, 2/4, 3/4 quantiles
- Four-way partition: < p1 | [p1, p2) | [p2, p3) | >= p3
- Adaptive to data distribution, balanced partitions for uniform/concentrated/skewed data
- Minimal overhead (32 samples sorting is negligible)

Time complexity:
- Best: O(n) - sorted data
- Average: O(n log n)
- Worst: O(n^2) - specially constructed data

Space complexity: O(log n) recursion stack
"""

import sys
import random

sys.setrecursionlimit(100000)


class DASv2:
    """DAS Sort v2.0 - Quaternary partition with sampled quantiles"""
    
    SAMPLE_SIZE = 32
    
    def __init__(self):
        self.comparisons = 0
        self.swaps = 0
        
    def sort(self, data):
        self.comparisons = 0
        self.swaps = 0
        if len(data) > 1:
            self._sort(data, 0, len(data) - 1)
        return data
        
    def _insertion_sort(self, data, left, right):
        """Insertion sort for small arrays (n <= 16)"""
        for i in range(left + 1, right + 1):
            key = data[i]
            j = i - 1
            while j >= left:
                self.comparisons += 1
                if data[j] > key:
                    data[j + 1] = data[j]
                    self.swaps += 1
                    j -= 1
                else:
                    break
            data[j + 1] = key
            
    def _sample_quantiles(self, data, left, right):
        """
        Sample and estimate quantiles
        
        Returns:
            (p1, p2, p3): 1/4, 2/4, 3/4 quantiles
        """
        size = right - left + 1
        sample_count = min(self.SAMPLE_SIZE, size)
        
        if sample_count < 4:
            min_val = min(data[left:right+1])
            max_val = max(data[left:right+1])
            mid = (min_val + max_val) / 2
            q = (max_val - min_val) / 4
            return (min_val + q, mid, max_val - q)
        
        samples = []
        step = max(1, size // sample_count)
        for i in range(sample_count):
            idx = left + (i * step) % size
            samples.append(data[idx])
        samples.sort()
        
        p1 = samples[sample_count // 4]
        p2 = samples[sample_count // 2]
        p3 = samples[3 * sample_count // 4]
        
        return (p1, p2, p3)
        
    def _partition_two_way(self, data, left, right, pivot):
        """
        Two-way partition: < pivot | >= pivot
        
        Returns:
            Partition index, left side < pivot, right side >= pivot
        """
        i = left
        j = right
        while True:
            while i <= j:
                self.comparisons += 1
                if data[i] < pivot:
                    i += 1
                else:
                    break
            while i <= j:
                self.comparisons += 1
                if data[j] >= pivot:
                    j -= 1
                else:
                    break
            if i >= j:
                break
            data[i], data[j] = data[j], data[i]
            self.swaps += 1
            i += 1
            j -= 1
        return i
        
    def _four_way_partition(self, data, left, right, p1, p2, p3):
        """
        Four-way partition: < p1 | [p1, p2) | [p2, p3) | >= p3
        
        Implemented using three two-way partitions
        
        Returns:
            (b1, b2, b3): three boundary points
            Intervals: [left, b1), [b1, b2), [b2, b3), [b3, right]
        """
        m1 = self._partition_two_way(data, left, right, p2)
        
        if m1 > left:
            m0 = self._partition_two_way(data, left, m1 - 1, p1)
        else:
            m0 = left
            
        if m1 <= right:
            m2 = self._partition_two_way(data, m1, right, p3)
        else:
            m2 = m1
            
        return (m0, m1, m2)
        
    def _sort(self, data, left, right):
        """DAS Sort v2 core recursive function"""
        if left >= right:
            return
            
        size = right - left + 1
        
        if size <= 16:
            self._insertion_sort(data, left, right)
            return
            
        is_sorted = True
        for i in range(left, right):
            self.comparisons += 1
            if data[i] > data[i + 1]:
                is_sorted = False
                break
        if is_sorted:
            return
            
        p1, p2, p3 = self._sample_quantiles(data, left, right)
        
        if p1 == p2 == p3:
            min_val = min(data[left:right+1])
            max_val = max(data[left:right+1])
            pivot = (min_val + max_val) / 2
            i, j, k = left, left, right
            while j <= k:
                self.comparisons += 1
                if data[j] < pivot:
                    data[i], data[j] = data[j], data[i]
                    self.swaps += 1
                    i += 1
                    j += 1
                elif data[j] > pivot:
                    data[j], data[k] = data[k], data[j]
                    self.swaps += 1
                    k -= 1
                else:
                    j += 1
            self._sort(data, left, i - 1)
            self._sort(data, k + 1, right)
            return
            
        b1, b2, b3 = self._four_way_partition(data, left, right, p1, p2, p3)
        
        if b1 > left:
            self._sort(data, left, b1 - 1)
        if b2 > b1:
            self._sort(data, b1, b2 - 1)
        if b3 > b2:
            self._sort(data, b2, b3 - 1)
        if right >= b3:
            self._sort(data, b3, right)


if __name__ == "__main__":
    import time
    
    print("=" * 60)
    print("DAS Sort v2.0 - Quaternary partition with sampled quantiles")
    print("=" * 60)
    
    alg = DASv2()
    
    data = [random.randint(0, 10000) for _ in range(100000)]
    start = time.perf_counter()
    alg.sort(data)
    elapsed = time.perf_counter() - start
    
    is_correct = all(data[i] <= data[i+1] for i in range(len(data)-1))
    
    print(f"Data size: 100,000 (random)")
    print(f"Time: {elapsed*1000:.2f} ms")
    print(f"Comparisons: {alg.comparisons:,}")
    print(f"Swaps: {alg.swaps:,}")
    print(f"Sorted correctly: {'Yes' if is_correct else 'No'}")
    
    print("\n" + "=" * 60)
    print("Multi-scenario test")
    print("=" * 60)
    
    test_cases = [
        ("Sorted", list(range(10000))),
        ("Reverse", list(range(10000, 0, -1))),
        ("Duplicates", [random.randint(0, 10) for _ in range(10000)]),
        ("Normal", [int(random.gauss(5000, 1000)) for _ in range(10000)]),
        ("Skewed", [int(random.expovariate(0.001)) % 10000 for _ in range(10000)]),
    ]
    
    for name, test_data in test_cases:
        original = test_data.copy()
        start = time.perf_counter()
        alg.sort(test_data)
        elapsed = time.perf_counter() - start
        is_correct = all(test_data[i] <= test_data[i+1] for i in range(len(test_data)-1))
        print(f"{name:12} | Time: {elapsed*1000:8.2f} ms | Correct: {'Yes' if is_correct else 'No'}")