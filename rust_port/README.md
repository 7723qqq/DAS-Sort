# DAS v1 — Rust 移植

初版 `das_sort_pure.cpp` (92 行核心) 的零 `unsafe` 安全 Rust 移植。

- `src/das_v1.rs` — 算法本体 + 单元测试 (含 std 排序 oracle 一致性校验)
- `src/main.rs` — 基准: 对比 Rust 标准库 `sort()` (driftsort/Timsort)
  与 `sort_unstable()` (ipnsort/pdqsort)
- `BENCH_RESULTS.md` — 最近一次 CI runner 上的实测结果 (由 CI 自动写入)

运行:

```bash
cargo test --release
cargo run --release
```

## 实测结论 (rustc 1.98.1, GitHub Actions runner)

| n = 1M | das_v1 | std sort (driftsort) | sort_unstable (ipnsort) |
|--------|--------|----------------------|-------------------------|
| Sorted    | **0.13ms** 🥇 | 0.56ms | 0.56ms |
| Reverse   | 0.88ms | 0.66ms | **0.65ms** 🥇 |
| Random    | 87.9ms | 20.0ms | **20.6ms** 🥇 |
| AlmostSort| **10.3ms** 🥇 | 28.7ms | 13.2ms |
| OrganPipe | 16.1ms | **3.71ms** 🥇 | 16.7ms |

要点 (诚实版, 含比较注意事项):

1. **完美有序上 v1 反而赢现代 Rust std 4.3 倍** (0.13 vs 0.56ms)。
   driftsort 的 run 检测状态机比 v1 的裸扫描开销大;
   另外 v1 用裸 `<` 比较, std 用 `total_cmp` (NaN 全序, 每次比较更贵)。
2. **AlmostSort 是 v1 在 Rust 上的意外杀手锏, 全场第一**
   (10.3 vs driftsort 的 28.7ms): 1000 次随机交换把数组切成 ~2000 个碎
   run, driftsort 需要约 11 趟归并; 而 v1 的值中点枢轴在近乎有序数据上
   切分近乎完美, 快排兜底反而更快。这与 C++ 上 Timsort 赢 v1 的结论相反
   —— 说明"自适应谁赢"取决于乱序的 *结构*, 没有普适答案。
3. **随机数据 4.3 倍差距原样存在** (87.9 vs 20.6ms) —— 与 C++ 对 pdqsort
   的 3.9 倍差距一致。无分支分区是跨语言的代差。
4. 公平性说明: std 保证 NaN 全序语义 (total_cmp), v1 用裸 `<` 不处理
   NaN —— v1 赢的场景有一部分来自"承诺更少"。两者在无 NaN 数据上
   语义一致。
5. 安全性: 整个移植零 `unsafe`, 不可能越界; C++ 初版的 1KB 固定栈在
   Rust 里只是最坏情况时间护栏, 不再涉及内存安全。
