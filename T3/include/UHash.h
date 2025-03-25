#pragma once
#include "includes.h"

template <typename T>
class UHash {
    private:
        int64_t a, b, p;
    public:
        UHash();
        int64_t operator() (T x);
};

#include "../src/UHash.tpp"