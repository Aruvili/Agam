import math

def main():
    limit = 100000
    count = 0
    
    for num in range(2, limit):
        is_prime = True
        max_i = int(math.isqrt(num))
        for i in range(2, max_i + 1):
            if num % i == 0:
                is_prime = False
                break
        if is_prime:
            count += 1
            
    print(count)

if __name__ == "__main__":
    main()
