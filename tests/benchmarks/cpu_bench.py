import time
import math

def cpu_test(n=10_000_000):
    start = time.perf_counter()
    total = 0.0
    for i in range(1, n + 1):
        total += math.sqrt(i) * math.sin(i)
    duration = time.perf_counter() - start
    print(f"Result: {total:.4f}")
    return duration

if __name__ == "__main__":
    print(f"CPU Test: {cpu_test():.4f} sec")
