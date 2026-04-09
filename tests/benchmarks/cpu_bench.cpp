#include <iostream>
#include <cmath>
#include <chrono>
#include <iomanip>

int main() {
    int n = 10000000;
    auto start = std::chrono::high_resolution_clock::now();
    double total = 0.0;
    for (int i = 1; i <= n; ++i) {
        total += std::sqrt(i) * std::sin(i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Result: " << std::fixed << std::setprecision(4) << total << std::endl;
    std::cout << "CPU Test: " << std::fixed << std::setprecision(4) << diff.count() << " sec" << std::endl;
    return 0;
}
