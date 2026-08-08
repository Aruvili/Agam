#include <iostream>
#include <cmath>

int main() {
    int limit = 100000;
    int count = 0;
    
    for (int num = 2; num < limit; ++num) {
        bool is_prime = true;
        int max_i = static_cast<int>(std::sqrt(num));
        for (int i = 2; i <= max_i; ++i) {
            if (num % i == 0) {
                is_prime = false;
                break;
            }
        }
        if (is_prime) {
            count++;
        }
    }
    
    std::cout << count << std::endl;
    return 0;
}
