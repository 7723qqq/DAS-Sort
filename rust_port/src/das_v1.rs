//! DAS v1 — Density Adaptive Sort, 安全 Rust 移植版。
//!
//! 与 C++ 初版 (das_sort_pure.cpp) 相同的三步思想:
//!   1. 有序检测: O(n) 最佳情况
//!   2. pivot = (min + max) / 2  — 值中点枢轴 (选"值"而非选"元素")
//!   3. 两路分区 <= pivot | > pivot, 小数组插入排序
//!
//! Rust 移植的几点观察:
//!   - 零 unsafe: 全部安全索引, 不可能越界; C++ 的指针算术在这里不存在
//!   - slice::is_sorted() 是标准库内建原语 (C++ 里要手写 6 行)
//!   - 固定 128 槽显式栈 + 超限兜底插入排序, 与 C++ 初版一一对应
//!     (在 Rust 里这不是内存安全问题, 只是最坏情况时间护栏)
//!   - f64 没有 Ord (NaN), 本实现只用 < / <=, 语义与 C++ 完全一致:
//!     含 NaN 时视为未排序, 行为与 C++ 初版相同 (NaN 排到最后附近, 未定义区)

const INSERTION_THRESHOLD: usize = 16;
const MAX_STACK: usize = 128;

pub fn das_sort(data: &mut [f64]) {
    let n = data.len();
    if n <= 1 {
        return;
    }

    let mut stack: [(usize, usize); MAX_STACK] = [(0, 0); MAX_STACK];
    let mut top = 0;
    stack[top] = (0, n - 1);
    top += 1;

    while top > 0 {
        top -= 1;
        let (left, right) = stack[top];

        if right - left + 1 <= INSERTION_THRESHOLD {
            insertion_sort(&mut data[left..=right]);
            continue;
        }
        // 有序检测: Rust 标准库内建 (C++ 初版手写的 is_sorted_check)
        if data[left..=right].is_sorted() {
            continue;
        }

        let (mn, mx) = min_max(&data[left..=right]);
        if mn == mx {
            continue; // 全部相等
        }
        let pivot = (mn + mx) / 2.0;

        let split = partition(data, left, right, pivot);

        if top + 2 >= MAX_STACK {
            // 兜底: 最坏情况时间护栏 (与 C++ 初版相同)
            insertion_sort(&mut data[left..=right]);
            continue;
        }

        let has_left = split > left;
        let has_right = split < right;
        let l_size = split - left;
        let r_size = right + 1 - split;

        // 较大的一半后压栈 (先处理), 保持栈深 O(log n)
        if l_size > r_size {
            if has_right {
                stack[top] = (split, right);
                top += 1;
            }
            if has_left {
                stack[top] = (left, split - 1);
                top += 1;
            }
        } else {
            if has_left {
                stack[top] = (left, split - 1);
                top += 1;
            }
            if has_right {
                stack[top] = (split, right);
                top += 1;
            }
        }
    }
}

fn insertion_sort(s: &mut [f64]) {
    for i in 1..s.len() {
        let key = s[i];
        let mut j = i;
        while j > 0 && s[j - 1] > key {
            s[j] = s[j - 1];
            j -= 1;
        }
        s[j] = key;
    }
}

/// 两路分区: <= pivot | > pivot, 返回右半区起始索引
fn partition(data: &mut [f64], left: usize, right: usize, pivot: f64) -> usize {
    let mut i = left;
    let mut j = right;
    while i <= j {
        if data[i] <= pivot {
            i += 1;
        } else if data[j] > pivot {
            j -= 1;
        } else {
            data.swap(i, j);
            i += 1;
            j -= 1;
        }
    }
    i
}

fn min_max(s: &[f64]) -> (f64, f64) {
    let mut mn = s[0];
    let mut mx = s[0];
    for &x in &s[1..] {
        if x < mn {
            mn = x;
        }
        if x > mx {
            mx = x;
        }
    }
    (mn, mx)
}

#[cfg(test)]
mod tests {
    use super::*;

    fn is_sorted_by(s: &[f64]) -> bool {
        s.windows(2).all(|w| w[0] <= w[1])
    }

    fn assert_sorts(input: Vec<f64>) {
        let mut a = input.clone();
        das_sort(&mut a);
        assert!(is_sorted_by(&a), "not sorted");
        let mut b = input;
        b.sort_by(f64::total_cmp);
        assert_eq!(a, b, "differs from std oracle");
    }

    #[test]
    fn edge_cases() {
        for input in [
            vec![],
            vec![1.0],
            vec![2.0, 1.0],
            vec![1.0, 2.0],
            vec![7.0; 1000],
            (0..1000).map(|i| i as f64).collect(),
            (0..1000).map(|i| (1000 - i) as f64).collect(),
        ] {
            assert_sorts(input);
        }
    }

    #[test]
    fn duplicates_and_random() {
        assert_sorts((0..2000).map(|i| (i % 10) as f64).collect());
        let mut rng_state = 12345u64;
        let mut rand = move || {
            rng_state ^= rng_state << 13;
            rng_state ^= rng_state >> 7;
            rng_state ^= rng_state << 17;
            (rng_state % 1_000_000) as f64
        };
        assert_sorts((0..10_000).map(|_| rand()).collect());
        // 阈值边界: 恰好 16 / 32 / 17 个元素
        for n in [16usize, 17, 32, 33] {
            assert_sorts((0..n).map(|_| rand()).collect());
        }
    }
}
