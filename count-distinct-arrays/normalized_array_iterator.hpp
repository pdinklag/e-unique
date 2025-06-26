#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>

using UInt = uint8_t;

class NormalizedArrayIterator {
private:
    size_t n_;
    size_t sigma_;

    std::unique_ptr<UInt[]> array_;

    // increments the current array
    void increment() {
        size_t i = n_ - 1;
        while(array_[i] + 1 == sigma_) {
            array_[i] = 0;
            --i;
        }
        ++array_[i];
    }

    // tests if the current array is normalized
    bool is_normalized() const {
        size_t s = 0;
        for(size_t i = 0; i < n_; i++) {
            if(array_[i] == s) {
                ++s;
            } else if(array_[i] > s) {
                return false;
            }
        }
        return s == sigma_;
    }

public:
    NormalizedArrayIterator(size_t n, size_t sigma) : n_(n), sigma_(sigma) {
        assert(sigma > 1);
        assert(n >= sigma);

        array_ = std::make_unique<UInt[]>(n_);
        for(size_t i = 0; i < n_; i++) array_[i] = 0;
        next();
    }

    // test if the current array can ever become normalized again by incrementing it
    // once the (sigma-1)-th entry becomes sigma, that is no longer the case
    bool has_next() const {
        bool ascending_prefix = true;
        for(size_t c = 0; c < sigma_ - 2; c++) {
            if(array_[c] != c) {
                ascending_prefix = false;
                break;
            }
        }

        return !ascending_prefix || array_[sigma_ - 2] <= sigma_ - 2;
    }

    // advance to the next normalized array
    void next() {
        if(!has_next()) return;

        bool normalized;
        do {
            increment();
            normalized = is_normalized();
        } while(!normalized && has_next());
    }

    // exists unique in range?
    bool exists_unique(size_t const i, size_t const j) const {
        assert(j < n_);
        assert(i <= j);

        if(i == j)[[unlikely]] return true;

        size_t freq[sigma_];
        for(size_t c = 0; c < sigma_; c++) {
            freq[c] = 0;
        }

        for(size_t x = i; x <= j; x++) {
            ++freq[array_[x]];
        }

        for(size_t c = 0; c < sigma_; c++) {
            if(freq[c] == 1) return true;
        }
        return false;
    }

    template<typename OutputStream>
    void print(OutputStream& out) const {
        out << "[";
        for(size_t i = 0; i < n_; i++) {
            out << size_t(array_[i] + 1);
            if(i + 1 < n_) out << ",";
        }
        out << "]";
    }
};
