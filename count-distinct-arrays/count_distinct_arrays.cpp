#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>

#include "normalized_array_iterator.hpp"

using Signature = __uint128_t;

int main(int argc, char** argv) {
    if(argc == 3) {
        auto const n = std::stoull(argv[1]);
        auto const sigma = std::stoull(argv[2]);

        if(n > 16) {
            std::cerr << "n must be at most 16" << std::endl;
            return -2;
        }

        size_t count = 0, count_distinct = 0;
        NormalizedArrayIterator it(n, sigma);
        std::unordered_map<Signature, size_t> distinct_arrays;

        do {
            ++count;
            
            Signature signature = 0;
            for(size_t i = 0; i < n - 1; i++) {
                for(size_t j = i + 1; j < n; j++) {
                    signature = (signature << 1) | it.exists_unique(i, j);
                }
            }

            std::cout << "#" << std::setfill(' ') << std::setw(8) << count << ": ";
            it.print(std::cout);
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

            it.next();
        } while(it.has_next());

        std::cout << "number of normalized arrays for n=" << n << " and sigma=" << sigma << ": " << count << std::endl;
        std::cout << "distinct: " << count_distinct << std::endl;

        return 0;
    } else {
        std::cerr << "Usage: " << argv[0] << " <n> <sigma>" << std::endl;
        return -1;
    }
}
