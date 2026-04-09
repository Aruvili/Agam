#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <numeric>

int main() {
    int n = 5000000;
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<long long> arr(n);
    for (int i = 0; i < n; ++i) {
        arr[i] = (long long)i * 2;
    }
    long long total = std::accumulate(arr.begin(), arr.end(), 0LL);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Result: " << total << std::endl;
    std::cout << "Array Test: " << std::fixed << std::setprecision(4) << diff.count() << " sec" << std::endl;
    return 0;
}
