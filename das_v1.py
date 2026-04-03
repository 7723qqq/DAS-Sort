#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
DAS Sort v1.0 - 原版
作者: 原创
日期: 2026

核心思想:
- 使用 (min + max) / 2 作为枢轴值，而不是选择某个具体元素
- 三向分区: < pivot | = pivot | > pivot
- 自适应检测已排序数据 (O(n) 最佳情况)
- 小数组使用插入排序优化

时间复杂度:
- 最佳: O(n) - 已排序数据
- 平均: O(n log n)
- 最差: O(n²) - 特殊构造数据

空间复杂度: O(log n) 递归栈
"""

import sys

sys.setrecursionlimit(50000)


class DASv1:
    """DAS Sort v1.0 - 原版二分分区"""
    
    def __init__(self):
        self.comparisons = 0
        self.swaps = 0
        
    def sort(self, data):
        """
        排序入口函数
        
        Args:
            data: 待排序的列表
            
        Returns:
            排序后的列表 (原地排序)
        """
        self.comparisons = 0
        self.swaps = 0
        self._sort(data, 0, len(data) - 1)
        return data
        
    def _insertion_sort(self, data, left, right):
        """插入排序 - 用于小数组优化 (n <= 16)"""
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
            
    def _sort(self, data, left, right):
        """DAS Sort 核心递归函数"""
        if left >= right:
            return
            
        size = right - left + 1
        
        # 小数组优化: 使用插入排序
        if size <= 16:
            self._insertion_sort(data, left, right)
            return
            
        # 已排序检测: O(n) 最佳情况
        is_sorted = True
        for i in range(left, right):
            self.comparisons += 1
            if data[i] > data[i + 1]:
                is_sorted = False
                break
        if is_sorted:
            return
            
        # 计算密度自适应枢轴: (min + max) / 2
        min_val = min(data[left:right+1])
        max_val = max(data[left:right+1])
        pivot = (min_val + max_val) / 2
        
        # 三向分区: [ < pivot | = pivot | > pivot ]
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
                
        # 递归排序左右两部分 (= pivot 部分已经有序)
        self._sort(data, left, i - 1)
        self._sort(data, k + 1, right)


if __name__ == "__main__":
    import random
    import time
    
    print("=" * 60)
    print("DAS Sort v1.0 - 原版测试")
    print("=" * 60)
    
    alg = DASv1()
    
    # 测试随机数据
    data = [random.randint(0, 10000) for _ in range(100000)]
    start = time.perf_counter()
    alg.sort(data)
    elapsed = time.perf_counter() - start
    
    is_correct = all(data[i] <= data[i+1] for i in range(len(data)-1))
    
    print(f"数据量: 100,000 (随机)")
    print(f"耗时: {elapsed*1000:.2f} ms")
    print(f"比较次数: {alg.comparisons:,}")
    print(f"交换次数: {alg.swaps:,}")
    print(f"排序正确: {'✓' if is_correct else '✗'}")
