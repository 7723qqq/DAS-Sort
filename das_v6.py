#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
DAS Sort v6.0 - Run-Adaptive Hybrid (实验版)
作者: 原创
日期: 2026

核心思想 (借鉴 Timsort 的 run 概念, 与 DAS 快排形成混合):
- 阶段 1 run 检测: 单趟扫描找出极大升序段, 严格降序段原地反转为升序段
- 阶段 2 路径选择:
    * runs == 1  -> 已整体有序 (或反序已反转), O(n) 直接返回
    * runs <= 16 -> 自然归并: 相邻 run 两两归并, O(n·log R), 且稳定
    * 否则       -> v2 式四向分区自适应快排兜底, O(n log n)
- 显式栈实现 (无递归, 无递归深度限制)

阈值 RUN_THRESHOLD = 16 的依据 (实测): 归并每趟的每元素代价高于快排分区,
墙钟时间交叉点约在 R ≈ 16 (log2(16) = 4 趟); run 更多时快排兜底更快。

适用场景:
- 大胜: 已排序 / 逆序 / 风琴管 / 拼接的有序段 / 突发乱序块 (run 结构数据)
- 持平: 交换扰动型近乎有序、随机数据 (走快排兜底, 比纯 v2 多一趟检测开销)

时间复杂度:
- 最佳: O(n)        - 已排序 / 逆序
- 典型: O(n·log R)  - R = run 数量, R 小时远优于 O(n log n)
- 最差: O(n log n)  - 乱序数据走快排兜底

空间复杂度: O(log n) 显式栈 + 归并临时缓冲 O(n)

稳定性: 归并路径稳定 (相等元素保持原始相对顺序); 快排兜底路径不稳定。
"""

import random


class DASv6:
    """DAS Sort v6.0 - Run-Adaptive Hybrid"""

    # run 数量 <= RUN_THRESHOLD 时走自然归并, 否则快排兜底
    # (实测墙钟时间交叉点; 归并趟数 = ceil(log2 R))
    RUN_THRESHOLD = 16

    # 快排兜底参数 (与 v2 保持一致)
    SAMPLE_SIZE = 32

    def __init__(self):
        self.comparisons = 0
        self.swaps = 0  # 元素移动次数 (含归并写入)

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
        n = len(data)

        if n <= 1:
            return data
        if n <= 16:
            self._insertion_sort(data, 0, n - 1)
            return data

        # 阶段 1: run 检测 (降序段原地反转)
        runs = self._detect_runs(data, n)

        # 阶段 2: 路径选择
        if len(runs) == 1:
            return data  # 已整体有序

        if len(runs) <= self.RUN_THRESHOLD:
            # run 结构数据: 自然归并, O(n·log R), 稳定
            self._merge_all(data, runs)
            return data

        # 乱序: v2 式四向分区快排兜底
        self._quick_sort(data, 0, n - 1)
        return data

    # ---------- 阶段 1: run 检测 ----------

    def _detect_runs(self, data, n):
        """
        单趟扫描找出极大升序段; 严格降序段原地反转为升序段

        Returns:
            runs: [(start, end), ...] 升序段列表 (闭区间), 严格按位置递增
        """
        runs = []
        i = 0
        while i < n:
            j = i
            if j + 1 >= n:
                runs.append((i, j))
                break
            self.comparisons += 1
            if data[j] <= data[j + 1]:
                # 升序段
                while j + 1 < n and data[j] <= data[j + 1]:
                    self.comparisons += 1
                    j += 1
            else:
                # 严格降序段: 反转 (严格降序无重复, 反转不影响稳定性)
                while j + 1 < n and data[j] > data[j + 1]:
                    self.comparisons += 1
                    j += 1
                self._reverse_array(data, i, j)
            runs.append((i, j))
            i = j + 1
        return runs

    def _reverse_array(self, data, left, right):
        """数组反转 - O(n/2)"""
        while left < right:
            data[left], data[right] = data[right], data[left]
            self.swaps += 2
            left += 1
            right -= 1

    # ---------- 阶段 2a: 自然归并 ----------

    def _merge_all(self, data, runs):
        """相邻 run 两两归并, 直到只剩一个 run"""
        while len(runs) > 1:
            merged = []
            for i in range(0, len(runs) - 1, 2):
                s1, e1 = runs[i]
                s2, e2 = runs[i + 1]
                self._merge_runs(data, s1, e1, s2, e2)
                merged.append((s1, e2))
            if len(runs) % 2:
                merged.append(runs[-1])
            runs = merged

    def _merge_runs(self, data, start1, end1, start2, end2):
        """原地归并两个相邻升序段 [start1..end1] 与 [start2..end2] (稳定)"""
        temp = data[start1:end1 + 1]
        len_t = len(temp)
        i = 0
        j = start2
        k = start1
        while i < len_t and j <= end2:
            self.comparisons += 1
            if data[j] < temp[i]:
                data[k] = data[j]
                j += 1
            else:
                # 相等时取左段 -> 稳定
                data[k] = temp[i]
                i += 1
            self.swaps += 1
            k += 1
        while i < len_t:
            data[k] = temp[i]
            i += 1
            k += 1
            self.swaps += 1
        while j <= end2:
            data[k] = data[j]
            j += 1
            k += 1
            self.swaps += 1

    # ---------- 阶段 2b: v2 式快排兜底 (显式栈) ----------

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

    def _sample_quantiles(self, data, left, right):
        """采样 32 个元素, 估计 1/4、2/4、3/4 分位数"""
        size = right - left + 1
        sample_count = min(self.SAMPLE_SIZE, size)

        if sample_count < 4:
            min_val = min(data[left:right + 1])
            max_val = max(data[left:right + 1])
            mid = (min_val + max_val) / 2
            q = (max_val - min_val) / 4
            return (min_val + q, mid, max_val - q)

        step = max(1, size // sample_count)
        samples = []
        for i in range(sample_count):
            idx = left + (i * step) % size
            samples.append(data[idx])
        samples.sort()

        p1 = samples[sample_count // 4]
        p2 = samples[sample_count // 2]
        p3 = samples[3 * sample_count // 4]
        return (p1, p2, p3)

    def _partition_two_way(self, data, left, right, pivot):
        """两路分区: < pivot | >= pivot, 返回分界索引"""
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
        """四向分区: < p1 | [p1, p2) | [p2, p3) | >= p3 (三次两路分区)"""
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

    def _quick_sort(self, data, left, right):
        """v2 式四向分区快排 (显式栈实现, 无递归)"""
        stack = [(left, right)]
        while stack:
            left, right = stack.pop()
            if left >= right:
                continue

            size = right - left + 1

            if size <= 16:
                self._insertion_sort(data, left, right)
                continue

            is_sorted = True
            for i in range(left, right):
                self.comparisons += 1
                if data[i] > data[i + 1]:
                    is_sorted = False
                    break
            if is_sorted:
                continue

            p1, p2, p3 = self._sample_quantiles(data, left, right)

            if p1 == p2 == p3:
                # 采样全相同: 用 (min+max)/2 做三向分区
                min_val = min(data[left:right + 1])
                max_val = max(data[left:right + 1])
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
                if left < i - 1:
                    stack.append((left, i - 1))
                if k + 1 < right:
                    stack.append((k + 1, right))
                continue

            b1, b2, b3 = self._four_way_partition(data, left, right, p1, p2, p3)

            if b1 > left:
                stack.append((left, b1 - 1))
            if b2 > b1:
                stack.append((b1, b2 - 1))
            if b3 > b2:
                stack.append((b2, b3 - 1))
            if right >= b3:
                stack.append((b3, right))


if __name__ == "__main__":
    import time

    print("=" * 64)
    print("DAS Sort v6.0 - Run-Adaptive Hybrid 测试")
    print("=" * 64)

    from v2.das_v2 import DASv2

    def make_nearly_sorted(n, k, rng):
        data = list(range(n))
        for _ in range(k):
            a = rng.randrange(n)
            b = rng.randrange(n)
            data[a], data[b] = data[b], data[a]
        return data

    def make_concat_segments(n, k, rng):
        seg = n // k
        order = list(range(k))
        rng.shuffle(order)
        out = []
        for o in order:
            out.extend(o * seg + i for i in range(seg))
        return out

    rng = random.Random(42)
    cases = [
        ("完全有序 (100k)", list(range(100000))),
        ("完全逆序 (100k)", list(range(100000, 0, -1))),
        ("风琴管 (100k)", [*range(50000), *range(50000, 0, -1)]),
        ("拼接有序段 8段 (100k)", make_concat_segments(100000, 8, rng)),
        ("近乎有序 100处错位 (100k)", make_nearly_sorted(100000, 100, rng)),
        ("近乎有序 1000处错位 (100k)", make_nearly_sorted(100000, 1000, rng)),
        ("随机浮点 (100k)", [rng.random() for _ in range(100000)]),
    ]

    for name, data in cases:
        expect = sorted(data)

        d6 = data.copy()
        alg6 = DASv6()
        t0 = time.perf_counter()
        alg6.sort(d6)
        t6 = time.perf_counter() - t0
        ok6 = d6 == expect

        d2 = data.copy()
        alg2 = DASv2()
        t0 = time.perf_counter()
        alg2.sort(d2)
        t2 = time.perf_counter() - t0
        ok2 = d2 == expect

        print(f"\n--- {name} ---")
        print(f"  v6: {t6*1000:9.2f} ms | 比较 {alg6.comparisons:>9,} | 正确 {ok6}")
        print(f"  v2: {t2*1000:9.2f} ms | 比较 {alg2.comparisons:>9,} | 正确 {ok2}")
