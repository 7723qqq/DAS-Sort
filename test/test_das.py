import pytest
import sys
import os
import random

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from das_v1 import DASv1
from das_v2 import DASv2


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


class TestCompare:
    def test_v1_v2_same_result(self):
        for trial in range(10):
            data1 = generate_random(1000, seed=trial)
            data2 = data1.copy()
            
            v1 = DASv1()
            v2 = DASv2()
            v1.sort(data1)
            v2.sort(data2)
            
            assert is_sorted(data1)
            assert is_sorted(data2)
            assert data1 == data2


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
