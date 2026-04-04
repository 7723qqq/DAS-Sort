"""
Example: Log Timestamp Sorting

This example demonstrates sorting log entries by timestamp,
a common scenario where data is often nearly sorted.
"""

import sys
import os
import random
import time
from datetime import datetime, timedelta

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from das_v2 import DASv2


def generate_log_entries(count, out_of_order_ratio=0.001):
    """Generate log entries with some out-of-order timestamps"""
    base_time = datetime(2024, 1, 1, 0, 0, 0)
    entries = []
    
    for i in range(count):
        timestamp = base_time + timedelta(seconds=i)
        entries.append({
            'timestamp': timestamp,
            'level': random.choice(['INFO', 'WARN', 'ERROR']),
            'message': f'Log entry {i}'
        })
    
    for _ in range(int(count * out_of_order_ratio)):
        a = random.randint(0, count - 1)
        b = random.randint(0, count - 1)
        entries[a], entries[b] = entries[b], entries[a]
    
    return entries


def main():
    print("=" * 60)
    print("Example: Log Timestamp Sorting")
    print("=" * 60)
    
    for count in [10000, 100000]:
        print(f"\nLog entries: {count:,}")
        
        entries = generate_log_entries(count)
        timestamps = [e['timestamp'].timestamp() for e in entries]
        
        sorter = DASv2()
        start = time.perf_counter()
        sorter.sort(timestamps)
        t_das = time.perf_counter() - start
        
        timestamps2 = [e['timestamp'].timestamp() for e in entries]
        start = time.perf_counter()
        timestamps2.sort()
        t_builtin = time.perf_counter() - start
        
        print(f"  DAS v2:    {t_das*1000:.2f} ms")
        print(f"  Built-in:  {t_builtin*1000:.2f} ms")


if __name__ == "__main__":
    main()
