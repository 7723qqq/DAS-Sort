#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
DAS Sort v5.0 - 终极优化版
作者: 原创
日期: 2026

基于 v1.0 的改进:
- 三分区创新: 将数据分成三等分 (bound1, bound2) 而非单一枢轴
- 逆序检测与降级: 检测到逆序直接反转 O(n/2)
- 重复数据优化: 少量唯一值使用计数排序 O(n)
- 自适应分界点: 根据数据分布动态调整分区边界
- 小数组优化: n <= 16 使用插入排序
- 已排序检测: O(n) 最佳情况
- 显式栈实现 (无递归, 无递归深度限制)

时间复杂度:
- 最佳: O(n) - 已排序 / 逆序 / 重复数据
- 平均: O(n log n) - 随机数据 (三分区 log₃n 深度)
- 最差: O(n²) - 特殊构造数据

空间复杂度: O(log n) 显式栈
"""


class DASv5:
    """DAS Sort v5.0 - 终极优化版"""
    
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
            
    def _reverse_array(self, data, left, right):
        """数组反转 - 用于逆序数据降级 O(n/2)"""
        while left < right:
            data[left], data[right] = data[right], data[left]
            self.swaps += 1
            left += 1
            right -= 1
            
    def _try_counting_sort(self, data, left, right):
        """
        少量唯一值优化 - 计数排序思想 O(n)

        仅当唯一值数量 <= min(1024, size/4) 时执行 (超过则 dict 开销逼近
        快排, 放弃); 唯一值超限立即早退, 避免对连续浮点等全唯一数据白白
        构建大 dict。

        Returns:
            bool: 是否已完成排序
        """
        size = right - left + 1
        max_unique = min(1024, size // 4)
        count = {}
        for i in range(left, right + 1):
            v = data[i]
            count[v] = count.get(v, 0) + 1
            if len(count) > max_unique:
                return False  # 唯一值过多, 交回快排

        idx = left
        for val in sorted(count.keys()):
            for _ in range(count[val]):
                if data[idx] != val:
                    self.swaps += 1
                data[idx] = val
                idx += 1
        return True
            
    def _sort(self, data, left, right):
        """DAS Sort v5.0 核心函数 (显式栈实现, 无递归)"""
        stack = [(left, right)]
        while stack:
            left, right = stack.pop()
            if left >= right:
                continue
                
            size = right - left + 1
            
            # 小数组优化
            if size <= 16:
                self._insertion_sort(data, left, right)
                continue
                
            # 已排序检测: O(n)
            is_sorted = True
            for i in range(left, right):
                self.comparisons += 1
                if data[i] > data[i + 1]:
                    is_sorted = False
                    break
            if is_sorted:
                continue
                
            # 逆序检测与降级: O(n)
            is_reverse = True
            for i in range(left, right):
                self.comparisons += 1
                if data[i] < data[i + 1]:
                    is_reverse = False
                    break
            if is_reverse:
                self._reverse_array(data, left, right)
                continue
                
            # 计算三分区的两个分界点
            min_val = min(data[left:right+1])
            max_val = max(data[left:right+1])
            range_val = max_val - min_val
            
            if range_val == 0:
                continue
                
            # 重复数据检测: 少量唯一值用计数排序
            if range_val < size / 10 and size > 1000:
                if self._try_counting_sort(data, left, right):
                    continue
                
            # 自适应分界点: 根据数据分布调整
            sample_size = min(size, 100)
            sample_step = size // sample_size if size > sample_size else 1
            sample = [data[left + i * sample_step] for i in range(min(sample_size, size))]
            sample.sort()
            
            median = sample[len(sample) // 2]
            expected_median = (min_val + max_val) / 2
            
            # 如果数据分布不均匀，使用中位数调整分界点
            if abs(median - expected_median) > range_val * 0.2:
                bound1 = (min_val + median) / 2
                bound2 = (median + max_val) / 2
            else:
                # 数据分布均匀，标准三分区
                bound1 = min_val + range_val / 3      # 1/3 分界点
                bound2 = min_val + 2 * range_val / 3  # 2/3 分界点
            
            # 三向分区: [ <bound1 | [bound1,bound2] | >bound2 ]
            i = left
            j = left
            k = right
            
            while j <= k:
                self.comparisons += 1
                if data[j] < bound1:
                    data[i], data[j] = data[j], data[i]
                    self.swaps += 1
                    i += 1
                    j += 1
                elif data[j] > bound2:
                    data[j], data[k] = data[k], data[j]
                    self.swaps += 1
                    k -= 1
                else:
                    j += 1
            
            # 子区间压栈
            stack.append((left, i - 1))    # 下区 (< bound1)
            stack.append((i, k))           # 中区 ([bound1, bound2])
            stack.append((k + 1, right))   # 上区 (> bound2)


if __name__ == "__main__":
    import random
    import time
    
    print("=" * 60)
    print("DAS Sort v5.0 - 终极版测试")
    print("=" * 60)
    
    alg = DASv5()
    
    test_cases = [
        ("随机数据", lambda n: [random.randint(0, 10000) for _ in range(n)]),
        ("完全有序", lambda n: list(range(n))),
        ("完全逆序", lambda n: list(range(n, 0, -1))),
        ("少量重复", lambda n: [random.choice([100,500,1000,2000]) for _ in range(n)]),
    ]
    
    for name, gen_fn in test_cases:
        data = gen_fn(100000)
        start = time.perf_counter()
        alg.sort(data)
        elapsed = time.perf_counter() - start
        
        is_correct = all(data[i] <= data[i+1] for i in range(len(data)-1))
        
        print(f"\n--- {name} ---")
        print(f"耗时: {elapsed*1000:.2f} ms")
        print(f"比较次数: {alg.comparisons:,}")
        print(f"交换次数: {alg.swaps:,}")
        print(f"排序正确: {'✓' if is_correct else '✗'}")
