#include <cassert>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

using UInt = uint8_t;
using Signature = __uint128_t;

int main(int argc, char** argv) {
    if(argc == 3) {
        size_t const n = std::stoull(argv[1]);
        size_t const sigma = std::stoull(argv[2]);

        if(n > 16) {
            std::cerr << "n must be at most 16" << std::endl;
            return -2;
        }
        if(sigma < 2) {
            std::cerr << "sigma must be at least 2" << std::endl;
            return -2;            
        }
        if(sigma > n) {
            std::cerr << "sigma must be at most n" << std::endl;
            return -2;
        }

        size_t count = 0, count_distinct = 0;
        // NormalizedArrayIterator it(n, sigma);
        auto array = std::make_unique<UInt[]>(n);
        std::unordered_map<Signature, size_t> distinct_arrays;

        auto exists_unique = [&](size_t const i, size_t const j){
            assert(i < j);
            if(i == j)[[unlikely]] return true;
            size_t freq[sigma];
            for(size_t c = 0; c < sigma; c++) freq[c] = 0;
            for(size_t x = i; x <= j; x++) ++freq[array[x]];
            for(size_t c = 0; c < sigma; c++) { if(freq[c] == 1) return true; }
            return false;
        };

        auto print = [&](){
            std::cout << "[";
            for(size_t i = 0; i < n; i++) {
                std::cout << size_t(array[i] + 1);
                if(i + 1 < n) std::cout << ",";
            }
            std::cout << "]";
        };

        auto handle_next = [&](){
            ++count;
            
            Signature signature = 0;
            for(size_t i = 0; i < n - 1; i++) {
                for(size_t j = i + 1; j < n; j++) {
                    signature = (signature << 1) | exists_unique(i, j);
                }
            }

            std::cout << "#" << std::setfill(' ') << std::setw(8) << count << ": ";
            print();
            std::cout << " -> 0x" << std::hex << std::setw(8) << std::setfill('0') << uint64_t(signature >> 64) << std::setw(8) << uint64_t(signature) << std::dec;

            auto x = distinct_arrays.find(signature);
            if(x != distinct_arrays.end()) {
                std::cout << " (= #" << x->second << ")";
            } else {
                ++count_distinct;
                distinct_arrays.emplace(signature, count);

                std::cout << " (*)";
            }
            std::cout << std::endl;
        };

        std::function<void(size_t, size_t)> enumerate = [&](size_t const i, size_t const s){
            if(i == n) {
                // handle
                handle_next();
            } else {
                // recurse
                auto const slots_remaining = n - i;
                auto const missing = sigma - s;

                auto const x_min = (slots_remaining == missing) ? s : 0;
                auto const x_max = std::min(s, sigma-1);

                for(size_t x = x_min; x <= x_max; x++) {
                    array[i] = x;
                    enumerate(i+1, x < x_max ? s : s+1);
                }
            }
        };

        enumerate(0, 0);

        std::cout << "number of normalized arrays for n=" << n << " and sigma=" << sigma << ": " << count << std::endl;
        std::cout << "distinct: " << count_distinct << std::endl;

        return 0;
    } else {
        std::cerr << "Usage: " << argv[0] << " <n> <sigma>" << std::endl;
        return -1;
    }
}
