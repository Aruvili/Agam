#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <iomanip>

bool is_prime(int num) {
    if (num < 2) return false;
    int limit = static_cast<int>(std::sqrt(num));
    for (int i = 2; i <= limit; ++i) {
        if (num % i == 0) return false;
    }
    return true;
}

int main() {
    int limit = 20000;
    auto start = std::chrono::high_resolution_clock::now();
    int count = 0;
    for (int num = 2; num < limit; ++num) {
        if (is_prime(num)) {
            count++;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Result: " << count << std::endl;
    std::cout << "Prime Test: " << std::fixed << std::setprecision(4) << diff.count() << " sec" << std::endl;
    return 0;
}
