# Rust 移植基准结果 (GitHub Actions runner)

- 运行时间: 2026-09-18 07:55 UTC
- 工具链: rustc 1.98.1 (48a229cea 2026-09-01)

```

===== n = 100000 =====
Scenario         das_v1   std sort   unstable
----------------------------------------------
Sorted           0.01ms     0.06ms     0.05ms
Reverse          0.09ms     0.07ms     0.06ms
Random           7.17ms     1.93ms     1.89ms
AlmostSort       0.70ms     2.05ms     1.12ms
OrganPipe        1.38ms     0.44ms     1.40ms

===== n = 1000000 =====
Scenario         das_v1   std sort   unstable
----------------------------------------------
Sorted           0.13ms     0.56ms     0.56ms
Reverse          0.88ms     0.66ms     0.65ms
Random          87.87ms    20.02ms    20.62ms
AlmostSort      10.27ms    28.67ms    13.18ms
OrganPipe       16.05ms     3.71ms    16.67ms

Note: 7 runs truncated mean. std sort = stable (driftsort on 1.81+,
Timsort before); sort_unstable = ipnsort on 1.81+ (pdqsort before).
```
