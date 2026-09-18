import pytest
import sys
import os
import random

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from das_v1 import DASv1
from v2.das_v2 import DASv2
from das_v5 import DASv5


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


class TestCompare:
    def test_v1_v2_v5_same_result(self):
        for trial in range(10):
            data1 = generate_random(1000, seed=trial)
            data2 = data1.copy()
            data5 = data1.copy()

            v1 = DASv1()
            v2 = DASv2()
            v5 = DASv5()
            v1.sort(data1)
            v2.sort(data2)
            v5.sort(data5)

            assert is_sorted(data1)
            assert is_sorted(data2)
            assert is_sorted(data5)
            assert data1 == data2
            assert data1 == data5


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
