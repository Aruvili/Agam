import time

def array_test(n=5_000_000):
    start = time.perf_counter()
    arr = [i * 2 for i in range(n)]
    total = sum(arr)
    duration = time.perf_counter() - start
    print(f"Result: {total}")
    return duration

if __name__ == "__main__":
    print(f"Array Test: {array_test():.4f} sec")
