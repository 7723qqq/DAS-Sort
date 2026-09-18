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
