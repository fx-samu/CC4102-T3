#include "includes.h"

template <typename T>
UHash<T>::UHash(): a(0), p((1ULL << 61) - 1) {
    std::random_device rd;
    std::mt19937_64 rng(rd());
    while (!a) 
        a = rng() % p;
    b = rng() % p;
}

template <typename T>
int64_t UHash<T>::operator()(T x) {
    int64_t hx = static_cast<int64_t>(x); 
    int64_t result = (a * hx + b) % p;
    if (result < 0) 
        result += p;
    return result;
}