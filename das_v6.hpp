#ifndef DAS_V6_HPP
#define DAS_V6_HPP

#include <vector>
#include <algorithm>
#include <utility>
#include <cstdint>

// DAS v6 - Run-Adaptive Hybrid (experimental)
//
// Timsort-style run detection layered on top of the DAS quicksort core:
//   Stage 1 (run detection): one pass finds maximal ascending runs; strictly
//   descending runs are reversed in place (no duplicates in a strictly
//   descending run, so stability is unaffected).
//   Stage 2 (path selection):
//     - runs == 1        -> already sorted (or reversed), O(n), done
//     - runs <= 16       -> natural merge: pairwise merging of adjacent runs,
//                           O(n*log R). This path is stable.
//     - otherwise        -> v2-style four-way adaptive quicksort fallback
//   The quicksort fallback uses an explicit stack (no recursion).
//
// RUN_THRESHOLD rationale: each merge pass costs more per element than a
// quicksort partition pass; the wall-clock crossover measured at ~16 runs
// (log2(16) = 4 merge passes). Above that, the quicksort fallback wins.
//
// Complexity:
//   Best:  O(n)        - sorted / reversed input
//   Typical: O(n*log R) - R = number of runs (R small => near O(n))
//   Worst: O(n log n)  - unsorted input takes the quicksort fallback
//
// Stability: the merge path is stable (equal keys keep their original order).
// Stability is unobservable for plain double keys; use the Python version
// (das_v6.py) if you need it. The quicksort fallback path is NOT stable.
//
// Like the rest of this repository, the implementation is specialized for
// std::vector<double>. API mirrors DASv2.
class DASv6 {
private:
    static constexpr int INSERTION_THRESHOLD = 16;
    static constexpr int SAMPLE_SIZE = 32;
    static constexpr int RUN_THRESHOLD = 16;

    int64_t comparisons_;
    int64_t swaps_;
    std::vector<double> merge_buffer_;  // reused across merges (no realloc churn)

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

    void reverseRange(double* arr, int left, int right) {
        while (left < right) {
            std::swap(arr[left], arr[right]);
            swaps_ += 2;
            ++left;
            --right;
        }
    }

    // Stage 1: find maximal ascending runs; reverse descending runs in place.
    // Returns [(start, end), ...] with strictly increasing positions.
    std::vector<std::pair<int, int>> detectRuns(double* arr, int n) {
        std::vector<std::pair<int, int>> runs;
        int i = 0;
        while (i < n) {
            int j = i;
            if (j + 1 >= n) {
                runs.emplace_back(i, j);
                break;
            }
            ++comparisons_;
            if (arr[j] <= arr[j + 1]) {
                // ascending run
                while (j + 1 < n) {
                    ++comparisons_;
                    if (arr[j] <= arr[j + 1]) {
                        ++j;
                    } else {
                        break;
                    }
                }
            } else {
                // strictly descending run: reverse it
                while (j + 1 < n) {
                    ++comparisons_;
                    if (arr[j] > arr[j + 1]) {
                        ++j;
                    } else {
                        break;
                    }
                }
                reverseRange(arr, i, j);
            }
            runs.emplace_back(i, j);
            i = j + 1;
        }
        return runs;
    }

    // Merge adjacent ascending runs [s1..e1] and [s2..e2] (s2 == e1 + 1).
    // Ties take the left run first -> stable.
    void mergeRuns(double* arr, int s1, int e1, int s2, int e2) {
        int len1 = e1 - s1 + 1;
        if ((int)merge_buffer_.size() < len1) merge_buffer_.resize(len1);
        for (int i = 0; i < len1; ++i) merge_buffer_[i] = arr[s1 + i];

        int i = 0;
        int j = s2;
        int k = s1;
        while (i < len1 && j <= e2) {
            ++comparisons_;
            if (arr[j] < merge_buffer_[i]) {
                arr[k++] = arr[j++];
            } else {
                arr[k++] = merge_buffer_[i++];
            }
            ++swaps_;
        }
        while (i < len1) {
            arr[k++] = merge_buffer_[i++];
            ++swaps_;
        }
        while (j <= e2) {
            arr[k++] = arr[j++];
            ++swaps_;
        }
    }

    // Stage 2a: pairwise-merge runs until a single run remains.
    void mergeAll(double* arr, std::vector<std::pair<int, int>>& runs) {
        while (runs.size() > 1) {
            std::vector<std::pair<int, int>> merged;
            merged.reserve((runs.size() + 1) / 2);
            for (size_t i = 0; i + 1 < runs.size(); i += 2) {
                mergeRuns(arr, runs[i].first, runs[i].second,
                          runs[i + 1].first, runs[i + 1].second);
                merged.emplace_back(runs[i].first, runs[i + 1].second);
            }
            if (runs.size() % 2) merged.push_back(runs.back());
            runs = std::move(merged);
        }
    }

    // ---------- Stage 2b: v2-style four-way quicksort fallback ----------

    void sampleQuantiles(const double* arr, int left, int right,
                         double& p1, double& p2, double& p3) {
        int size = right - left + 1;
        int sampleCount = std::min(SAMPLE_SIZE, size);

        if (sampleCount < 4) {
            double minVal = arr[left];
            double maxVal = arr[left];
            for (int i = left + 1; i <= right; ++i) {
                ++comparisons_;
                if (arr[i] < minVal) minVal = arr[i];
                if (arr[i] > maxVal) maxVal = arr[i];
            }
            double mid = (minVal + maxVal) / 2.0;
            double q = (maxVal - minVal) / 4.0;
            p1 = minVal + q;
            p2 = mid;
            p3 = maxVal - q;
            return;
        }

        std::vector<double> samples;
        samples.reserve(sampleCount);
        int step = std::max(1, size / sampleCount);
        for (int i = 0; i < sampleCount; ++i) {
            int idx = left + (i * step) % size;
            samples.push_back(arr[idx]);
        }
        std::sort(samples.begin(), samples.end());

        p1 = samples[sampleCount / 4];
        p2 = samples[sampleCount / 2];
        p3 = samples[3 * sampleCount / 4];
    }

    int partitionTwoWay(double* arr, int left, int right, double pivot) {
        int i = left;
        int j = right;
        while (true) {
            while (i <= j) {
                ++comparisons_;
                if (arr[i] < pivot) {
                    ++i;
                } else {
                    break;
                }
            }
            while (i <= j) {
                ++comparisons_;
                if (arr[j] >= pivot) {
                    --j;
                } else {
                    break;
                }
            }
            if (i >= j) break;
            std::swap(arr[i], arr[j]);
            ++swaps_;
            ++i;
            --j;
        }
        return i;
    }

    void fourWayPartition(double* arr, int left, int right,
                          double p1, double p2, double p3,
                          int& b1, int& b2, int& b3) {
        int m1 = partitionTwoWay(arr, left, right, p2);

        if (m1 > left) {
            b1 = partitionTwoWay(arr, left, m1 - 1, p1);
        } else {
            b1 = left;
        }

        if (m1 <= right) {
            b3 = partitionTwoWay(arr, m1, right, p3);
        } else {
            b3 = m1;
        }

        b2 = m1;
    }

    void quickSort(double* arr, int left, int right) {
        std::vector<std::pair<int, int>> stack;
        stack.emplace_back(left, right);

        while (!stack.empty()) {
            left = stack.back().first;
            right = stack.back().second;
            stack.pop_back();

            if (left >= right) continue;
            int size = right - left + 1;

            if (size <= INSERTION_THRESHOLD) {
                insertionSort(arr, left, right);
                continue;
            }

            bool isSorted = true;
            for (int i = left; i < right; ++i) {
                ++comparisons_;
                if (arr[i] > arr[i + 1]) {
                    isSorted = false;
                    break;
                }
            }
            if (isSorted) continue;

            double p1, p2, p3;
            sampleQuantiles(arr, left, right, p1, p2, p3);

            if (p1 == p2 && p2 == p3) {
                // Sampled quantiles identical: three-way partition on (min+max)/2
                double minVal = arr[left];
                double maxVal = arr[left];
                for (int i = left + 1; i <= right; ++i) {
                    ++comparisons_;
                    if (arr[i] < minVal) minVal = arr[i];
                    if (arr[i] > maxVal) maxVal = arr[i];
                }
                double pivot = (minVal + maxVal) / 2.0;

                int i = left, j = left, k = right;
                while (j <= k) {
                    ++comparisons_;
                    if (arr[j] < pivot) {
                        std::swap(arr[i], arr[j]);
                        ++swaps_;
                        ++i;
                        ++j;
                    } else if (arr[j] > pivot) {
                        std::swap(arr[j], arr[k]);
                        ++swaps_;
                        --k;
                    } else {
                        ++j;
                    }
                }
                if (left < i - 1) stack.emplace_back(left, i - 1);
                if (k + 1 < right) stack.emplace_back(k + 1, right);
                continue;
            }

            int b1, b2, b3;
            fourWayPartition(arr, left, right, p1, p2, p3, b1, b2, b3);

            if (b1 > left) stack.emplace_back(left, b1 - 1);
            if (b2 > b1) stack.emplace_back(b1, b2 - 1);
            if (b3 > b2) stack.emplace_back(b2, b3 - 1);
            if (right >= b3) stack.emplace_back(b3, right);
        }
    }

public:
    DASv6() : comparisons_(0), swaps_(0) {}

    void sort(std::vector<double>& data) {
        comparisons_ = 0;
        swaps_ = 0;

        int n = static_cast<int>(data.size());
        if (n <= 1) return;
        double* arr = data.data();

        if (n <= INSERTION_THRESHOLD) {
            insertionSort(arr, 0, n - 1);
            return;
        }

        auto runs = detectRuns(arr, n);

        if (runs.size() <= 1) return;  // already sorted

        if ((int)runs.size() <= RUN_THRESHOLD) {
            mergeAll(arr, runs);
            return;
        }

        quickSort(arr, 0, n - 1);
    }

    int64_t getComparisons() const { return comparisons_; }
    int64_t getSwaps() const { return swaps_; }
};

#endif
