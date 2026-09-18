//! DAS v1 — 惯用 Rust (idiomatic Rust) 版本。
//!
//! 与 [`crate::das_v1`] (C 直译版) 算法完全相同:
//! 有序检测 O(n) -> 值中点枢轴 -> 两路分区 -> 显式栈迭代。
//!
//! 与直译版的区别 (本模块存在的意义):
//!
//! | 维度 | das_v1 (直译) | das_idiomatic (本模块) |
//! |------|--------------|------------------------|
//! | API | 自由函数 `das_sort(&mut v)` | extension trait: `v.das_sort()` |
//! | min/max | 手写循环 | `iter().copied().fold()` 单趟 |
//! | 栈 | 固定数组 `[(usize,usize); 128]` + 手动 top | `Vec` + `push/pop` |
//! | 分区 | 全局下标索引 | 子切片上的局部索引 + [`slice::swap`] 原语 |
//! | 文档 | 模块级 | 函数级 `///` + 复杂度/语义契约 |
//!
//! 关于分区原语的注记: stable Rust 并没有提供原位分区 (in-place
//! partition) 的标准原语 —— `partition_in_place` 至今仍是 nightly-only
//! 实验特性 (rust-lang/rust#62543)。std 自己的 sort 内部靠 unsafe 实现
//! 分区。因此"地道"的安全 Rust 只能做到: 分区逻辑局限在子切片上
//! (让边界检查易于被 LLVM 消除) 并使用 `swap` 原语, 而非全局索引。
//!
//! 为什么不做成泛型 `<T: Ord>`: v1 的身份是 *值中点* 枢轴 `(min+max)/2`,
//! 需要算术运算; 纯 std 无 num-traits 时无法对任意数值类型泛型化。
//! 值中点 + f64 是这个算法的契约的一部分 (语义与 C++ 初版一致,
//! 含 NaN 时行为同 C++ 初版: NaN 参与比较恒为 false, 属于契约外输入)。

/// DAS v1 排序, 惯用 Rust API 形状 (extension trait)。
pub trait DasSort {
    /// 原地排序 (升序, 非稳定)。
    ///
    /// # 算法
    /// 1. 单趟 [`slice::is_sorted`] 检测 -> 已有序则 O(n) 返回
    /// 2. pivot = (min + max) / 2 (值中点, 非元素)
    /// 3. 两路分区 <= pivot | > pivot (子切片 + `swap`, 无 nightly 原语)
    /// 4. 较大的一半后出栈 (先处理), 显式栈深度 O(log n)
    ///
    /// # 复杂度
    /// - 最佳 O(n) (已有序 / 全相等)
    /// - 最坏 O(n^2) (特殊构造输入; 栈内存仍为 O(log n))
    ///
    /// # Panics
    /// 无 (纯安全代码; 所有切片范围由不变量保证)。
    fn das_sort(&mut self);
}

impl DasSort for [f64] {
    fn das_sort(&mut self) {
        let n = self.len();
        if n <= 1 {
            return;
        }

        let mut stack = Vec::with_capacity(64);
        stack.push((0, n - 1));

        while let Some((left, right)) = stack.pop() {
            let subslice = &mut self[left..=right];

            if subslice.len() <= 16 {
                insertion_sort(subslice);
                continue;
            }
            if subslice.is_sorted() {
                continue;
            }

            let (mn, mx) = subslice
                .iter()
                .copied()
                .fold((f64::INFINITY, f64::NEG_INFINITY), |(mn, mx), x| {
                    (mn.min(x), mx.max(x))
                });
            if mn == mx {
                continue; // 全部相等
            }
            let pivot = (mn + mx) / 2.0;

            // 两路分区 (子切片局部索引 + swap): 返回 <= pivot 的元素个数
            let count = partition_le(subslice, pivot);
            let split = left + count;

            // 较大的一半后压栈 (先处理), 栈深 O(log n)
            if split - left > right + 1 - split {
                if split < right {
                    stack.push((split, right));
                }
                if split > left {
                    stack.push((left, split - 1));
                }
            } else {
                if split > left {
                    stack.push((left, split - 1));
                }
                if split < right {
                    stack.push((split, right));
                }
            }
        }
    }
}

/// 两路分区: <= pivot 的元素换到前段, 返回其数量 (单趟, 前段保序)
fn partition_le(s: &mut [f64], pivot: f64) -> usize {
    let mut i = 0;
    for j in 0..s.len() {
        if s[j] <= pivot {
            s.swap(i, j);
            i += 1;
        }
    }
    i
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

#[cfg(test)]
mod tests {
    use super::*;

    fn assert_sorts(input: Vec<f64>) {
        let expected = {
            let mut e = input.clone();
            e.sort_by(f64::total_cmp);
            e
        };
        let mut a = input.clone();
        a.das_sort();
        assert!(a.windows(2).all(|w| w[0] <= w[1]), "not sorted");
        assert_eq!(a, expected, "differs from std oracle");

        // 与直译版交叉验证: 两个移植必须产出逐位相同的结果
        let mut b = input;
        crate::das_v1::das_sort(&mut b);
        assert_eq!(a, b, "differs from the C-style port");
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
    fn random_and_thresholds() {
        let mut rng_state = 999u64;
        let mut rand = move || {
            rng_state ^= rng_state << 13;
            rng_state ^= rng_state >> 7;
            rng_state ^= rng_state << 17;
            (rng_state % 1_000_000) as f64
        };
        assert_sorts((0..10_000).map(|_| rand()).collect());
        for n in [16usize, 17, 63, 64, 65] {
            assert_sorts((0..n).map(|_| rand()).collect());
        }
    }
}
