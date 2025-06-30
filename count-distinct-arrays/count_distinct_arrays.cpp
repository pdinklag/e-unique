#include <cassert>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using UInt = uint8_t;
using Number = uint32_t;
using Signature = __uint128_t;

using Array = std::vector<UInt>;

bool exists_unique(Array const& array, size_t const sigma, size_t const i, size_t const j) {
    assert(i < j);
    if(i == j)[[unlikely]] return true;
    size_t freq[sigma];
    for(size_t c = 0; c < sigma; c++) freq[c] = 0;
    for(size_t x = i; x <= j; x++) ++freq[array[x]];
    for(size_t c = 0; c < sigma; c++) { if(freq[c] == 1) return true; }
    return false;
}

Signature compute_signature(Array const& array, size_t const sigma) {
    size_t const n = array.size();
    
    Signature signature = 0;
    for(size_t i = 0; i < n - 1; i++) {
        for(size_t j = i + 1; j < n; j++) {
            signature = (signature << 1) | exists_unique(array, sigma, i, j);
        }
    }
    return signature;
}

void print_array(Array const& array) {
    std::cout << "[";

    size_t const n = array.size();
    for(size_t i = 0; i < n; i++) {
        std::cout << size_t(array[i] + 1);
        if(i + 1 < n) std::cout << ",";
    }
    std::cout << "]";
}

void print_array_numbered(Array const& array, size_t const i) {
    std::cout << "#" << i << ": ";
    print_array(array);
}

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
        Array array(n);
        std::vector<Array> arrays;
        std::unordered_map<Signature, std::vector<Number>> eq_classes;

        std::function<void(size_t, size_t)> enumerate = [&](size_t const i, size_t const s){
            if(i == n) {
                // handle
                ++count;
                arrays.push_back(array);

                auto const signature = compute_signature(array, sigma);
                auto it = eq_classes.find(signature);
                if(it != eq_classes.end()) {
                    it->second.push_back(count);
                } else {
                    ++count_distinct;
                    eq_classes.emplace(signature, std::vector{Number(count)});
                }
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

        auto print_result = [&](){
            std::cout << "number of normalized arrays for n=" << n << " and sigma=" << sigma << ": " << count << std::endl;
            std::cout << "equivalence classes: " << count_distinct << std::endl;
        };

        auto print_eq = [&](std::vector<Number> const& cls){
            auto const m = cls.size();
            std::cout << "{" << std::endl;
            for(size_t i = 0; i < cls.size(); i++) {
                std::cout << "\t#" << cls[i] << ": ";
                print_array(arrays[cls[i]-1]);
                std::cout << std::endl;
            }
            std::cout << "}" << std::endl;
        };

        print_result();

        std::string cmd;
        do {
            std::cout << std::endl;
            std::cout << "Enter command (\"help\" for a list of commands, \"q\" to quit): ";
            std::getline(std::cin, cmd);
            std::cout << cmd << std::endl;

            if(cmd == "q" || cmd == "quit" || cmd == "") {
                return 0;
            } if(cmd == "help") {
                std::setfill(' ');
                std::cout << std::setw(20) << "help: " << "show this help" << std::endl;
                std::cout << std::setw(20) << "result: " << "show main result" << std::endl;
                std::cout << std::setw(20) << "all: " << "print all arrays" << std::endl;
                std::cout << std::setw(20) << "diff <X> <Y>: " << "compare arrays #<X> and #<Y>" << std::endl;
                std::cout << std::setw(20) << "eq: " << "list all equivalence classes" << std::endl;
                std::cout << std::setw(20) << "eq <X>: " << "list equivalence class that #<X> belongs to" << std::endl;
                std::cout << std::setw(20) << "find <ARRAY>" << "find the given array" << std::endl;
            } else if(cmd == "all") {
                auto const m = arrays.size();
                for(size_t i = 0; i < m; i++) {
                    print_array_numbered(arrays[i], i+1);
                    std::cout << std::endl;
                }
            } else if(cmd == "eq") {
                for(auto it : eq_classes) {
                    print_eq(it.second);
                }
            } else if(cmd.starts_with("eq")) {
                Number x;
                if(sscanf(cmd.data(), "eq %u", &x) == 1) {
                    auto const& cls = eq_classes[compute_signature(arrays[x-1], sigma)];
                    print_eq(cls);
                }
            } else if(cmd.starts_with("diff")) {
                Number x, y;
                if(sscanf(cmd.data(), "diff %u %u", &x, &y) == 2) {
                    auto const a = arrays[x-1];
                    auto const b = arrays[y-1];
                    std::cout << "X = "; print_array_numbered(a, x); std::cout << std::endl;
                    std::cout << "Y = "; print_array_numbered(b, y); std::cout << std::endl;

                    bool any_differences = false;
                    for(size_t i = 0; i < n - 1; i++) {
                        for(size_t j = i + 1; j < n; j++) {
                            auto const ra = exists_unique(a, sigma, i, j);
                            auto const rb = exists_unique(b, sigma, i, j);

                            if(ra != rb) {
                                any_differences = true;
                                std::cout << "\tdifferent result for [" << (i+1) << "," << (j+1) << "] (" << ra << " for X, " << rb << " for Y)" << std::endl;
                            }
                        }
                    }

                    if(!any_differences) {
                        std::cout << "\tthe arrays are equivalent!" << std::endl;
                    }
                }
            } else if(cmd.starts_with("find")) {
                std::replace(cmd.begin(), cmd.end(), ',', ' ');

                char* p = cmd.data() + 4;
                size_t q;
                for(size_t i = 0; i < n; i++) {
                    array[i] = std::stoul(p, &q) - 1;
                    p += q;
                }

                bool found = false;
                for(size_t i = 0; i < count; i++) {
                    if(arrays[i] == array) {
                        found = true;
                        print_array_numbered(array, i+1); std::cout << std::endl;
                        break;
                    }
                }

                if(!found) {
                    std::cout << "array not found!" << std::endl;
                }
            } else if(cmd == "result") {
                print_result();
            } else {
                std::cout << "unknown command" << std::endl;
            }
        } while(true);
        return 0;
    } else {
        std::cerr << "Usage: " << argv[0] << " <n> <sigma>" << std::endl;
        return -1;
    }
}
