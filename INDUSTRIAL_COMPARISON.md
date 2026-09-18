# DAS vs 工业级排序实现 — 对比报告

> 本文档记录 DAS 与工业级排序实现在相同环境下的实测对比, 以及诚实的差距分析。
> 复现方法见 [benchmark/industrial_bench.cpp](benchmark/industrial_bench.cpp) 文件头。

## 测试环境

- **平台**: Debian GNU/Linux (sandbox), GCC (libstdc++), `-O2 -std=c++17`
- **数据**: `std::vector<double>`
- **计时**: 7 次运行取截尾均值 (去掉最快/最慢各 1/4), 每格先用 `std::sort` oracle 验证正确性
- **参测实现**:
  - `std::sort` — libstdc++ introsort (无预检测)
  - `std::stable_sort` — libstdc++ 归并 (恒定 O(n log n), 无自适应)
  - `pdqsort` — [orlp/pdqsort](https://github.com/orlp/pdqsort), 无分支块分区 + 模式击败;
    Rust 标准库 `sort_unstable` (2021-2025) 的同源算法
  - `timsort` — [gfx/cpp-TimSort v1.2.2](https://github.com/gfx/cpp-TimSort), Timsort 的 C++ 移植;
    即 Python `sorted()` / Java 对象排序的算法
  - `DASv2` / `DASv6` — 本仓库实现 (带比较/交换计数插桩)

## 结果: n = 100,000 (时间, ms, 越低越好)

| Scenario   | std::sort | std::stable | pdqsort | timsort | DASv2  | DASv6 |
|------------|-----------|-------------|---------|---------|--------|-------|
| Random     | 6.51      | 8.13        | **2.90**| 11.46   | 14.18  | 11.64 |
| Sorted     | 1.00      | 0.75        | 0.08    | **0.06**| 0.25   | 0.06  |
| Reverse    | 0.78      | 1.19        | 0.19    | **0.09**| 0.63   | 0.10  |
| AlmostSort | 1.00      | 0.97        | 0.49    | **0.25**| 1.29   | 0.96  |
| OrganPipe  | 7.57      | 0.97        | 3.00    | **0.18**| 4.76   | 0.23  |
| Sawtooth   | 3.32      | 0.89        | 2.39    | **0.44**| 5.52   | 0.57  |
| Duplicates | 3.75      | 4.93        | **0.89**| 6.33    | 3.78   | 3.97  |
| AllSame    | 1.14      | 0.74        | 0.10    | **0.06**| 0.25   | 0.06  |
| DescBlocks | 4.12      | 1.24        | 1.21    | **0.09**| 1.33   | 0.16  |
| ShufWindows| 4.31      | 4.87        | **2.51**| 5.68    | 8.61   | 8.96  |
| Gaussian   | 6.53      | 7.99        | **2.90**| 9.01    | 11.21  | 11.41 |

## 结果: n = 1,000,000

| Scenario   | std::sort | std::stable | pdqsort | timsort | DASv2   | DASv6   |
|------------|-----------|-------------|---------|---------|---------|---------|
| Random     | 80.55     | 103.42      | **34.01**| 111.31 | 132.17  | 131.60  |
| Sorted     | 13.84     | 14.48       | 1.05    | **0.85**| 2.30    | 1.06    |
| Reverse    | 10.55     | 19.00       | 2.17    | **1.18**| 6.44    | 1.46    |
| AlmostSort | 13.51     | 16.18       | 7.93    | **6.06**| 17.45   | 14.60   |
| OrganPipe  | 85.48     | 22.89       | 43.94   | **3.71**| 44.99   | 2.44    |
| Sawtooth   | 40.62     | 16.72       | 29.85   | **5.74**| 60.48   | 11.23   |
| Duplicates | 39.83     | 55.84       | **10.39**| 67.28 | 37.91   | 40.81   |
| AllSame    | 14.63     | 13.85       | 1.29    | **0.80**| 2.31    | 0.80    |
| DescBlocks | 48.83     | 18.47       | 13.27   | **1.18**| 13.99   | 2.08    |
| ShufWindows| 57.98     | 71.89       | **30.32**| 73.80 | 103.28  | 105.17  |
| Gaussian   | 78.44     | 102.10      | **33.85**| 110.33| 131.71  | 129.49  |

数据类型说明:

- **DescBlocks**: n/50 个严格降序块 (块内降序、块间值域升序) — run 检测会反转每块,
  剩 n/50 个升序 run, 超过 v6 的归并阈值, 强制走快排兜底。
  但值分位数枢轴恰好落在块边界上, 切分近乎零交换, 所以 DASv6 仍比 std::sort 快 25 倍。
- **ShufWindows**: 有序数组中嵌入 n/100 大小的乱序窗口 ×10 — 突发乱序。
- **AlmostSort**: 0.1% 随机位置交换。

## 诚实结论

### 1. 对 std::sort (无自适应): DAS 的主张成立

README 的核心卖点被验证: 在结构化/自适应数据上 DASv6 比 std::sort 快
**15-40 倍** (OrganPipe 33x, DescBlocks 26x, Sorted 17x, Reverse 8x)。
std::sort 没有 run 检测, 这是它设计和契约上的空白, DAS 填补了它。

### 2. 对 pdqsort (乱序/重复数据的前沿): DAS 落后 4 倍

随机数据上 pdqsort 的无分支块分区 (branchless block partition) 达到 2.90ms,
DASv6 是 11.64ms。我们验证过这不是插桩开销 (剥掉计数器后 11.33ms, 仅 ~3%),
而是分区循环的工程代差: pdqsort 用无条件交换 + 分支消除 + 预取,
DAS 用 1990 年代风格的分支式逐元素比较。重复数据上 pdqsort 的等键分区
(0.89ms) 同样远超 DAS (3.97ms)。

### 3. 对 Timsort (自适应数据的前沿): DASv6 全面小幅落败

这是最关键的对比。在 DASv6 的每一个优势场景上, Timsort 都同等或更好:

| 场景 | timsort | DASv6 | 差距 |
|------|---------|-------|------|
| OrganPipe | 0.18 | 0.23 | +28% |
| Sawtooth | 0.44 | 0.57 | +30% |
| DescBlocks | 0.09 | 0.16 | +78% |
| AlmostSort | 0.25 | 0.96 | 3.8x |
| ShufWindows | 5.68 | 8.96 | +58% |

原因有三, 都是 Timsort 30 年打磨出的机制, DASv6 没有:

- **minrun 与 run 扩展**: AlmostSort/ShufWindows 中孤立交换把数组切成上百个
  碎 run, 超过 v6 的归并阈值后直接退回快排; Timsort 用二分插入把短 run
  扩展到 minrun (32), 吸收孤立乱序, 保持 O(n) 级行为
- **galloping 归并**: 两个 run 的元素域错开时 (DescBlocks/Sawtooth),
  gallop 模式让归并退化为近似 memcpy, DASv6 是逐元素比较
- **稳定性内建**: Timsort 全路径稳定, DASv6 只有归并路径稳定

### 4. Python 侧差距更大 (解释器税)

同样的日志时间戳场景 (100K), Python 内置 `sorted()` (C 实现的 Timsort)
1.57ms, DASv6 纯 Python 版 17.29ms — 差 11 倍。纯 Python 排序库的定位
只能是教学。

### 5. 总结定位

> DAS v6 是 Timsort 前半部分 (run 检测 + 自然归并) 的独立再发明,
> 但缺少后半部分 (minrun + galloping) —— 而那正是 Timsort 在自适应
> 场景保持领先的机制。

工业界的现实格局:

- 乱序/重复数据: **pdqsort** (及其衍生) 是前沿, DAS 差 4 倍
- 自适应/结构化数据: **Timsort** (Python/Java/Android/Rust old-stable) 是前沿,
  DASv6 落后 20%-4x
- DAS 真正击败的只有"无自适应的 std::sort"和纯 Python 环境
  (`sorted()` 之外的选择)

### 6. 如果 DAS 想追平 (路线图)

按预期收益排序:

1. **minrun + run 扩展 (二分插入)** — 修复 AlmostSort/ShufWindows 的 3.8x 差距
2. **galloping 归并** — 缩小 Sawtooth/DescBlocks 的 30-80% 差距
3. **无分支分区 + 排除计数器** — 缩小随机数据 4x 差距 (pdqsort 路线)
4. 等键检测 (Duplicates 场景)
5. 完成以上后, DAS 在算法上就是 "Timsort + pdqsort 混合" ——
   这正是 2025 年 Rust 标准库 driftsort/ipnsort 和 C++ IPS4o 系列的演进方向
