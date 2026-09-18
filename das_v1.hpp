#ifndef DAS_V1_HPP
#define DAS_V1_HPP

#include <vector>
#include <algorithm>
#include <utility>
#include <cstdint>

// DAS v1 - Density Adaptive Sort, original version.
//
// Core idea:
//   - Pivot is the VALUE midpoint (min + max) / 2, not a selected element
//   - Two-way partition: <= pivot | > pivot
//   - Adaptive: is_sorted_check() detects sorted data in O(n)
//   - Small arrays: insertion sort (threshold 16)
//   - Explicit stack (no recursion) with larger-side-processed-last ordering,
//     so stack depth stays O(log n); a fallback insertion sort guards against
//     pathological stack growth
//
// Complexity:
//   - Best: O(n)    - sorted data
//   - Average: O(n log n)
//   - Worst: O(n^2) - specially constructed data (bounded by insertion-sort
//     fallback when the stack grows too deep)
class DASv1 {
private:
    static constexpr int INSERTION_THRESHOLD = 16;
    static constexpr int MAX_STACK_SIZE = 128;

    int64_t comparisons_;
    int64_t swaps_;

    void insertionSort(double* arr, int left, int right) {
        for (int i = left + 1; i <= right; ++i) {
            double key = arr[i];
            int j = i - 1;
            while (j >= left) {
                ++comparisons_;
                if (arr[j] > key) {
                    arr[j + 1] = arr[j];
                    ++swaps_;
                    --j;
                } else {
                    break;
                }
            }
            arr[j + 1] = key;
        }
    }

    bool isSortedCheck(const double* arr, int left, int right) {
        for (int i = left; i < right; ++i) {
            ++comparisons_;
            if (arr[i] > arr[i + 1]) return false;
        }
        return true;
    }

public:
    DASv1() : comparisons_(0), swaps_(0) {}

    void sort(std::vector<double>& data) {
        comparisons_ = 0;
        swaps_ = 0;

        int n = static_cast<int>(data.size());
        if (n <= 1) return;
        double* arr = data.data();

        int stack_left[MAX_STACK_SIZE];
        int stack_right[MAX_STACK_SIZE];
        int top = 0;

        stack_left[top] = 0;
        stack_right[top] = n - 1;
        ++top;

        while (top > 0) {
            --top;
            int left = stack_left[top];
            int right = stack_right[top];
            int size = right - left + 1;

            if (size <= 1) continue;
            if (size <= INSERTION_THRESHOLD) {
                insertionSort(arr, left, right);
                continue;
            }
            if (isSortedCheck(arr, left, right)) continue;

            double mn = arr[left];
            double mx = arr[left];
            for (int i = left + 1; i <= right; ++i) {
                ++comparisons_;
                if (arr[i] < mn) mn = arr[i];
                if (arr[i] > mx) mx = arr[i];
            }

            if (mn == mx) continue;  // all elements equal

            double pivot = (mn + mx) / 2.0;

            // Two-way partition: <= pivot | > pivot
            int pi = left;
            int pj = right;
            while (pi <= pj) {
                if (arr[pi] <= pivot) {
                    ++comparisons_;
                    ++pi;
                } else if (arr[pj] > pivot) {
                    ++comparisons_;
                    ++comparisons_;
                    --pj;
                } else {
                    ++comparisons_;
                    ++comparisons_;
                    std::swap(arr[pi], arr[pj]);
                    ++swaps_;
                    ++pi;
                    --pj;
                }
            }

            int left_end = pi - 1;
            int right_start = pi;

            if (top + 2 >= MAX_STACK_SIZE) {
                // Fallback for pathological input: guarantees termination
                insertionSort(arr, left, right);
                continue;
            }

            int left_size = left_end - left + 1;
            int right_size = right - right_start + 1;

            // Push larger side last so it is processed first (LIFO),
            // keeping stack depth O(log n)
            if (left_size > right_size) {
                if (right > right_start) {
                    stack_left[top] = right_start;
                    stack_right[top] = right;
                    ++top;
                }
                if (left_end > left) {
                    stack_left[top] = left;
                    stack_right[top] = left_end;
                    ++top;
                }
            } else {
                if (left_end > left) {
                    stack_left[top] = left;
                    stack_right[top] = left_end;
                    ++top;
                }
                if (right > right_start) {
                    stack_left[top] = right_start;
                    stack_right[top] = right;
                    ++top;
                }
            }
        }
    }

    int64_t getComparisons() const { return comparisons_; }
    int64_t getSwaps() const { return swaps_; }
};

#endif
