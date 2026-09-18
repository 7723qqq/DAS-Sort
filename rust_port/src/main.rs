//! DAS v1 Rust 移植 — 基准对比 Rust 标准库排序。
//!
//! 参测:
//!   das_v1 (直译版)   — C 风格移植: 自由函数 + 手写下标分区 + 固定栈
//!   das_v1 (idiomatic) — 惯用 Rust: extension trait + partition_in_place + Vec 栈
//!   slice::sort        — Rust 标准库稳定排序 (1.81+ 为 driftsort; 旧版为 Timsort)
//!   slice::sort_unstable — Rust 标准库非稳定排序 (1.81+ 为 ipnsort; 旧版为 pdqsort)
//!
//! 两组 DAS 的对比回答一个问题: 按惯用 Rust 语义重写 (标准库原语替代
//! 手写循环) 的代价是多少?
//!
//! 关键背景: C++ 的 std::sort 没有任何自适应 (有序数据 18 倍的空当),
//! 而 Rust 的 std::sort **本身就是自适应的** — 这正是本基准要验证的:
//! DAS v1 在 C++ 生态的生态位, 在 Rust 标准库里是否已经不存在了。

mod das_idiomatic;
mod das_v1;

use das_idiomatic::DasSort;

use std::time::Instant;

const RUNS: usize = 7;

fn measure<F: FnMut(&mut [f64])>(data: &[f64], mut f: F) -> f64 {
    let mut times = Vec::with_capacity(RUNS);
    for _ in 0..RUNS {
        let mut copy = data.to_vec();
        let start = Instant::now();
        f(&mut copy);
        let elapsed = start.elapsed().as_secs_f64();
        assert!(
            copy.windows(2).all(|w| w[0] <= w[1]),
            "sorter produced unsorted output"
        );
        times.push(elapsed);
    }
    times.sort_by(f64::total_cmp);
    let trim = RUNS / 4;
    let sum: f64 = times[trim..RUNS - trim].iter().sum();
    (sum / (RUNS - 2 * trim) as f64) * 1000.0
}

fn main() {
    let mut rng_state = 42u64;
    let mut rand = move || {
        rng_state ^= rng_state << 13;
        rng_state ^= rng_state >> 7;
        rng_state ^= rng_state << 17;
        rng_state
    };

    for n in [100_000usize, 1_000_000] {
        println!("\n===== n = {n} =====");
        println!(
            "{:<12} {:>10} {:>10} {:>10} {:>10}",
            "Scenario", "das_v1", "das_idio", "std sort", "unstable"
        );
        println!("{}", "-".repeat(56));

        let scenario = |name: &str, data: Vec<f64>| {
            let expected = {
                let mut e = data.clone();
                e.sort_by(f64::total_cmp);
                e
            };
            let mut check = data.clone();
            das_v1::das_sort(&mut check);
            assert_eq!(check, expected, "das_v1 correctness");
            let mut check2 = data.clone();
            check2.das_sort();
            assert_eq!(check2, expected, "das_idiomatic correctness");

            let t_das = measure(&data, |v| das_v1::das_sort(v));
            let t_idio = measure(&data, |v| v.das_sort());
            let t_stable = measure(&data, |v| v.sort_by(f64::total_cmp));
            let t_unstable = measure(&data, |v| v.sort_unstable_by(f64::total_cmp));

            println!(
                "{:<12} {:>8.2}ms {:>8.2}ms {:>8.2}ms {:>8.2}ms",
                name, t_das, t_idio, t_stable, t_unstable
            );
        };

        {
            let mut d = Vec::with_capacity(n);
            for i in 0..n {
                d.push(i as f64);
            }
            scenario("Sorted", d);
        }
        {
            let mut d = Vec::with_capacity(n);
            for i in 0..n {
                d.push((n - i) as f64);
            }
            scenario("Reverse", d);
        }
        {
            let d: Vec<f64> = (0..n)
                .map(|_| (rand() % (u32::MAX as u64)) as f64 / 1000.0)
                .collect();
            scenario("Random", d);
        }
        {
            let mut d: Vec<f64> = (0..n).map(|i| i as f64).collect();
            for _ in 0..n / 1000 {
                let a = (rand() as usize) % n;
                let b = (rand() as usize) % n;
                d.swap(a, b);
            }
            scenario("AlmostSort", d);
        }
        {
            let mut d = Vec::with_capacity(n);
            for i in 0..n / 2 {
                d.push(i as f64);
            }
            for i in (0..n / 2).rev() {
                d.push(i as f64);
            }
            scenario("OrganPipe", d);
        }
    }

    println!(
        "\nNote: 7 runs truncated mean. das_v1 = C-style port, das_idio =\n\
         idiomatic rewrite (trait + partition_in_place + Vec stack).\n\
         std sort = stable (driftsort on 1.81+, Timsort before);\n\
         sort_unstable = ipnsort on 1.81+ (pdqsort before)."
    );
}
