import time
import math

def prime_test(limit=20000):
    start = time.perf_counter()
    primes = []
    for num in range(2, limit):
        is_prime = True
        for i in range(2, int(num**0.5) + 1):
            if num % i == 0:
                is_prime = False
                break
        if is_prime:
            primes.append(num)
    duration = time.perf_counter() - start
    print(f"Result: {len(primes)}")
    return duration

if __name__ == "__main__":
    print(f"Prime Test: {prime_test():.4f} sec")
