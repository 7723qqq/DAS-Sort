#ifndef DAS_V2_HPP
#define DAS_V2_HPP

#include <vector>
#include <algorithm>
#include <random>
#include <cstdint>

class DASv2 {
private:
    static constexpr int SAMPLE_SIZE = 32;
    static constexpr int INSERTION_THRESHOLD = 16;
    
    int64_t comparisons = 0;
    int64_t swaps = 0;
    
    std::mt19937 rng{std::random_device{}()};
    
    void insertionSort(std::vector<double>& data, int left, int right) {
        for (int i = left + 1; i <= right; ++i) {
            double key = data[i];
            int j = i - 1;
            while (j >= left) {
                ++comparisons;
                if (data[j] > key) {
                    data[j + 1] = data[j];
                    ++swaps;
                    --j;
                } else {
                    break;
                }
            }
            data[j + 1] = key;
        }
    }
    
    void sampleQuantiles(const std::vector<double>& data, int left, int right,
                         double& p1, double& p2, double& p3) {
        int size = right - left + 1;
        int sampleCount = std::min(SAMPLE_SIZE, size);
        
        if (sampleCount < 4) {
            double minVal = data[left];
            double maxVal = data[left];
            for (int i = left + 1; i <= right; ++i) {
                if (data[i] < minVal) minVal = data[i];
                if (data[i] > maxVal) maxVal = data[i];
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
            samples.push_back(data[idx]);
        }
        std::sort(samples.begin(), samples.end());
        
        p1 = samples[sampleCount / 4];
        p2 = samples[sampleCount / 2];
        p3 = samples[3 * sampleCount / 4];
    }
    
    int partitionTwoWay(std::vector<double>& data, int left, int right, double pivot) {
        int i = left;
        int j = right;
        while (true) {
            while (i <= j) {
                ++comparisons;
                if (data[i] < pivot) {
                    ++i;
                } else {
                    break;
                }
            }
            while (i <= j) {
                ++comparisons;
                if (data[j] >= pivot) {
                    --j;
                } else {
                    break;
                }
            }
            if (i >= j) break;
            std::swap(data[i], data[j]);
            ++swaps;
            ++i;
            --j;
        }
        return i;
    }
    
    void fourWayPartition(std::vector<double>& data, int left, int right,
                          double p1, double p2, double p3,
                          int& b1, int& b2, int& b3) {
        int m1 = partitionTwoWay(data, left, right, p2);
        
        if (m1 > left) {
            b1 = partitionTwoWay(data, left, m1 - 1, p1);
        } else {
            b1 = left;
        }
        
        if (m1 <= right) {
            b3 = partitionTwoWay(data, m1, right, p3);
        } else {
            b3 = m1;
        }
        
        b2 = m1;
    }
    
    void sortImpl(std::vector<double>& data, int left, int right) {
        if (left >= right) return;
        
        int size = right - left + 1;
        
        if (size <= INSERTION_THRESHOLD) {
            insertionSort(data, left, right);
            return;
        }
        
        bool isSorted = true;
        for (int i = left; i < right; ++i) {
            ++comparisons;
            if (data[i] > data[i + 1]) {
                isSorted = false;
                break;
            }
        }
        if (isSorted) return;
        
        double p1, p2, p3;
        sampleQuantiles(data, left, right, p1, p2, p3);
        
        if (p1 == p2 && p2 == p3) {
            double minVal = data[left];
            double maxVal = data[left];
            for (int i = left + 1; i <= right; ++i) {
                if (data[i] < minVal) minVal = data[i];
                if (data[i] > maxVal) maxVal = data[i];
            }
            double pivot = (minVal + maxVal) / 2.0;
            
            int i = left, j = left, k = right;
            while (j <= k) {
                ++comparisons;
                if (data[j] < pivot) {
                    std::swap(data[i], data[j]);
                    ++swaps;
                    ++i;
                    ++j;
                } else if (data[j] > pivot) {
                    std::swap(data[j], data[k]);
                    ++swaps;
                    --k;
                } else {
                    ++j;
                }
            }
            sortImpl(data, left, i - 1);
            sortImpl(data, k + 1, right);
            return;
        }
        
        int b1, b2, b3;
        fourWayPartition(data, left, right, p1, p2, p3, b1, b2, b3);
        
        if (b1 > left) sortImpl(data, left, b1 - 1);
        if (b2 > b1) sortImpl(data, b1, b2 - 1);
        if (b3 > b2) sortImpl(data, b2, b3 - 1);
        if (right >= b3) sortImpl(data, b3, right);
    }
    
public:
    void sort(std::vector<double>& data) {
        comparisons = 0;
        swaps = 0;
        if (data.size() > 1) {
            sortImpl(data, 0, static_cast<int>(data.size()) - 1);
        }
    }
    
    int64_t getComparisons() const { return comparisons; }
    int64_t getSwaps() const { return swaps; }
};

#endif