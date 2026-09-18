import pytest
import sys
import os
import random

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from das_v1 import DASv1
from v2.das_v2 import DASv2
from das_v5 import DASv5
from das_v6 import DASv6


def is_sorted(data):
    return all(data[i] <= data[i + 1] for i in range(len(data) - 1))


def generate_random(n, seed=42):
    random.seed(seed)
    return [random.randint(0, 100000) for _ in range(n)]


class TestDASv1:
    def test_empty_array(self):
        data = []
        sorter = DASv1()
        sorter.sort(data)
        assert data == []

    def test_single_element(self):
        data = [42.0]
        sorter = DASv1()
        sorter.sort(data)
        assert data == [42.0]

    def test_two_elements_sorted(self):
        data = [1.0, 2.0]
        sorter = DASv1()
        sorter.sort(data)
        assert is_sorted(data)

    def test_two_elements_reverse(self):
        data = [2.0, 1.0]
        sorter = DASv1()
        sorter.sort(data)
        assert is_sorted(data)

    def test_all_identical(self):
        data = [42.0] * 1000
        sorter = DASv1()
        sorter.sort(data)
        assert is_sorted(data)

    def test_already_sorted(self):
        data = list(range(1000))
        sorter = DASv1()
        sorter.sort(data)
        assert is_sorted(data)

    def test_reverse_sorted(self):
        data = list(range(1000, 0, -1))
        sorter = DASv1()
        sorter.sort(data)
        assert is_sorted(data)

    def test_random_data(self):
        data = generate_random(10000)
        sorter = DASv1()
        sorter.sort(data)
        assert is_sorted(data)

    def test_exactly_16_elements(self):
        data = generate_random(16)
        sorter = DASv1()
        sorter.sort(data)
        assert is_sorted(data)

    def test_exactly_32_elements(self):
        data = generate_random(32)
        sorter = DASv1()
        sorter.sort(data)
        assert is_sorted(data)

    def test_duplicates(self):
        data = [i % 10 for i in range(1000)]
        sorter = DASv1()
        sorter.sort(data)
        assert is_sorted(data)


class TestDASv2:
    def test_empty_array(self):
        data = []
        sorter = DASv2()
        sorter.sort(data)
        assert data == []

    def test_single_element(self):
        data = [42.0]
        sorter = DASv2()
        sorter.sort(data)
        assert data == [42.0]

    def test_two_elements_sorted(self):
        data = [1.0, 2.0]
        sorter = DASv2()
        sorter.sort(data)
        assert is_sorted(data)

    def test_two_elements_reverse(self):
        data = [2.0, 1.0]
        sorter = DASv2()
        sorter.sort(data)
        assert is_sorted(data)

    def test_all_identical(self):
        data = [42.0] * 1000
        sorter = DASv2()
        sorter.sort(data)
        assert is_sorted(data)

    def test_already_sorted(self):
        data = list(range(1000))
        sorter = DASv2()
        sorter.sort(data)
        assert is_sorted(data)

    def test_reverse_sorted(self):
        data = list(range(1000, 0, -1))
        sorter = DASv2()
        sorter.sort(data)
        assert is_sorted(data)

    def test_random_data(self):
        data = generate_random(10000)
        sorter = DASv2()
        sorter.sort(data)
        assert is_sorted(data)

    def test_exactly_16_elements(self):
        data = generate_random(16)
        sorter = DASv2()
        sorter.sort(data)
        assert is_sorted(data)

    def test_exactly_32_elements(self):
        data = generate_random(32)
        sorter = DASv2()
        sorter.sort(data)
        assert is_sorted(data)

    def test_duplicates(self):
        data = [i % 10 for i in range(1000)]
        sorter = DASv2()
        sorter.sort(data)
        assert is_sorted(data)

    def test_nearly_sorted(self):
        data = list(range(10000))
        random.seed(42)
        for _ in range(10):
            a = random.randint(0, 9999)
            b = random.randint(0, 9999)
            data[a], data[b] = data[b], data[a]
        sorter = DASv2()
        sorter.sort(data)
        assert is_sorted(data)


class TestDASv5:
    def test_empty_array(self):
        data = []
        sorter = DASv5()
        sorter.sort(data)
        assert data == []

    def test_single_element(self):
        data = [42.0]
        sorter = DASv5()
        sorter.sort(data)
        assert data == [42.0]

    def test_two_elements_sorted(self):
        data = [1.0, 2.0]
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_two_elements_reverse(self):
        data = [2.0, 1.0]
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_all_identical(self):
        data = [42.0] * 1000
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_already_sorted(self):
        data = list(range(1000))
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_reverse_sorted(self):
        data = list(range(1000, 0, -1))
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_random_data(self):
        data = generate_random(10000)
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_exactly_16_elements(self):
        data = generate_random(16)
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_exactly_32_elements(self):
        data = generate_random(32)
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_duplicates_few_unique(self):
        # Triggers the counting-sort path (range < size/10 and size > 1000)
        data = [i % 10 for i in range(2000)]
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_duplicates_many_unique(self):
        data = [i % 500 for i in range(5000)]
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_nearly_sorted(self):
        data = list(range(10000))
        random.seed(42)
        for _ in range(10):
            a = random.randint(0, 9999)
            b = random.randint(0, 9999)
            data[a], data[b] = data[b], data[a]
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_clustered_values(self):
        # Values clustered in a narrow band: stresses the value-range based
        # three-way partition (middle band must shrink every level)
        random.seed(7)
        data = [random.uniform(0.4999, 0.5001) for _ in range(5000)]
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_gaussian(self):
        random.seed(11)
        data = [random.gauss(0.0, 1.0) for _ in range(10000)]
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)

    def test_floats_continuous_range(self):
        # Narrow float range triggers the counting-sort gate but has
        # >1024 unique values -> early abort must hand off to quicksort
        random.seed(13)
        data = [random.uniform(0.0, 100.0) for _ in range(5000)]
        sorter = DASv5()
        sorter.sort(data)
        assert is_sorted(data)


class TestCompare:
    def test_v1_v2_v5_v6_same_result(self):
        for trial in range(10):
            data1 = generate_random(1000, seed=trial)
            data2 = data1.copy()
            data5 = data1.copy()
            data6 = data1.copy()

            DASv1().sort(data1)
            DASv2().sort(data2)
            DASv5().sort(data5)
            DASv6().sort(data6)

            assert is_sorted(data1)
            assert is_sorted(data2)
            assert is_sorted(data5)
            assert is_sorted(data6)
            assert data1 == data2
            assert data1 == data5
            assert data1 == data6


class _StableItem:
    """Compares by key only - used to verify merge-path stability."""
    __slots__ = ("key", "seq")

    def __init__(self, key, seq):
        self.key = key
        self.seq = seq

    def __lt__(self, other):
        return self.key < other.key

    def __le__(self, other):
        return self.key <= other.key

    def __gt__(self, other):
        return self.key > other.key

    def __eq__(self, other):
        return self.key == other.key


class TestDASv6:
    def test_empty_array(self):
        data = []
        sorter = DASv6()
        sorter.sort(data)
        assert data == []

    def test_single_element(self):
        data = [42.0]
        sorter = DASv6()
        sorter.sort(data)
        assert data == [42.0]

    def test_two_elements_sorted(self):
        data = [1.0, 2.0]
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_two_elements_reverse(self):
        data = [2.0, 1.0]
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_all_identical(self):
        data = [42.0] * 1000
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_already_sorted(self):
        data = list(range(1000))
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_reverse_sorted(self):
        data = list(range(1000, 0, -1))
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_random_data(self):
        data = generate_random(10000)
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_random_floats(self):
        random.seed(5)
        data = [random.uniform(-1e6, 1e6) for _ in range(5000)]
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_exactly_16_elements(self):
        data = generate_random(16)
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_exactly_32_elements(self):
        data = generate_random(32)
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_duplicates(self):
        data = [i % 3 for i in range(2000)]
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_organ_pipe_merge_path(self):
        # 2 runs -> natural merge path
        data = [*range(5000), *range(5000, 0, -1)]
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_concatenated_segments_merge_path(self):
        # 4 internally-sorted segments in shuffled order -> merge path
        data = []
        for seg in [3, 0, 2, 1]:
            data.extend(seg * 1000 + i for i in range(1000))
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_nearly_sorted(self):
        data = list(range(10000))
        random.seed(42)
        for _ in range(10):
            a = random.randint(0, 9999)
            b = random.randint(0, 9999)
            data[a], data[b] = data[b], data[a]
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_zigzag_fallback(self):
        # Many tiny runs -> quicksort fallback
        data = [i if i % 2 == 0 else 10000 - i for i in range(10000)]
        sorter = DASv6()
        sorter.sort(data)
        assert is_sorted(data)

    def test_merge_path_stability(self):
        # Equal keys must keep their original relative order on the
        # natural-merge (adaptive) path. Three ascending blocks with
        # overlapping key ranges -> exactly 3 runs -> merge path.
        items = []
        seq = 0
        for base in (0, 5, 10):
            for key in range(base, base + 10):
                for _ in range(100):
                    items.append(_StableItem(key, seq))
                    seq += 1
        original = items.copy()
        sorter = DASv6()
        sorter.sort(items)
        expected = sorted(original, key=lambda x: x.key)  # stable oracle
        assert [x.seq for x in items] == [x.seq for x in expected]
        assert sorter.comparisons < 3 * len(items)  # sanity: merge path taken


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
