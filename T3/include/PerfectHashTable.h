#pragma once
#include "includes.h"

template <typename T>
class innerPerfectHashTable {
    private:
        UHash<T> m_hash;
        std::vector<std::unique_ptr<T>> m_table;
    public:
        int m_n;
        innerPerfectHashTable(int t_n, std::vector<T> &t_v);
        bool search(const T &x);
};

template <typename T>
class PerfectHashTable {
    private:
        double m_c, m_k;
        int m_n;
        UHash<T> m_hash;
        std::vector<innerPerfectHashTable<T>> m_table;
        void _PerfectHashTable(double t_c, double t_k, std::vector<T> &t_v, double p = 0.001);

        // Time bench
        int64_t m_t_tot;
        std::vector<int64_t> m_t_secs;

        // fail and benchmark nan
        bool m_fail = false;
    public:
        PerfectHashTable(double t_c, double t_k, std::vector<T> &t_elements, double p = 0.001);
        bool search(const T &x);
        std::string benchmark();
};

#include "../src/PerfectHashTable.tpp"