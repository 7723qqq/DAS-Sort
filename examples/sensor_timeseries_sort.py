"""
Example: Sensor Time Series Data Sorting

This example demonstrates sorting sensor readings by time,
where data is typically nearly sorted with occasional delays.
"""

import sys
import os
import random
import time

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from v2.das_v2 import DASv2
from das_v6 import DASv6


def generate_sensor_data(count, delay_ratio=0.01):
    """Generate sensor data with some delayed readings"""
    data = []
    base_time = time.time()
    
    for i in range(count):
        timestamp = base_time + i * 0.1
        value = random.uniform(20.0, 30.0)
        data.append({'timestamp': timestamp, 'value': value})
    
    for _ in range(int(count * delay_ratio)):
        idx = random.randint(0, count - 1)
        delay = random.uniform(0.1, 5.0)
        data[idx]['timestamp'] -= delay
    
    return data


def main():
    print("=" * 60)
    print("Example: Sensor Time Series Data Sorting")
    print("=" * 60)
    
    for count in [10000, 100000]:
        print(f"\nSensor readings: {count:,}")
        
        data = generate_sensor_data(count)
        timestamps = [d['timestamp'] for d in data]
        TIMESTAMP_COPY = timestamps.copy()
        
        start = time.perf_counter()
        DASv2().sort(timestamps)
        t_das = time.perf_counter() - start

        start = time.perf_counter()
        DASv6().sort(timestamps)
        t_das6 = time.perf_counter() - start

        timestamps2 = TIMESTAMP_COPY
        start = time.perf_counter()
        timestamps2.sort()
        t_builtin = time.perf_counter() - start

        print(f"  DAS v2:    {t_das*1000:.2f} ms")
        print(f"  DAS v6:    {t_das6*1000:.2f} ms")
        print(f"  Built-in:  {t_builtin*1000:.2f} ms")


if __name__ == "__main__":
    main()
