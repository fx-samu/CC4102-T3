#include "PerfectHashTable.h"
#define DEBUG false

template <typename RetT, typename Func>
std::pair<int64_t, RetT> delta(Func f) {
    if (DEBUG) 
        return {0, f()};
    auto start = std::chrono::high_resolution_clock::now();
    RetT ret = f();
    auto end = std::chrono::high_resolution_clock::now();
    auto d = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    return {d, std::move(ret)}; 
}

template <typename T>
std::pair<double, double> stat_mean_dev(std::vector<T> &v){
    if (v.size() == 0)
        return {std::nan(""), std::nan("")};
    double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
    double sq_sum = std::accumulate(v.begin(), v.end(), 0.0, 
        [mean](double acc, double x) {
            return acc + (x - mean) * (x - mean);
        });
    double dev = std::sqrt(sq_sum / v.size());
    return {mean, dev};
}

template <typename T>
innerPerfectHashTable<T>::innerPerfectHashTable(int t_n, std::vector<T> &t_v): m_n(t_n), m_table(t_n) {
    // Vegas to a perfect hash function
    while (true) {
        std::vector<bool> mask(m_n, false);
        bool collision = false;
        m_hash = UHash<T>();
        for (T &x: t_v) {
            if (mask[m_hash(x) % m_n]) {
                collision = true;
                break;
            }
            mask[m_hash(x) % m_n] = true;
        }
        if (!collision) 
            break;
    }
    
    for (T &x: t_v)
        m_table[m_hash(x) % m_n] = std::make_unique<T>(x);
}

template <typename T>
bool innerPerfectHashTable<T>::search(const T &x){
    if (!m_n)
        return false;
    unsigned long long idx = m_hash(x) % m_n;
    if (m_table[idx])
        return *m_table[idx] == x;
    return false;
}

template <typename T>
void PerfectHashTable<T>::_PerfectHashTable(double t_c, double t_k, std::vector<T> &t_v, double p) {
    m_c = t_c; m_k = t_k; m_n = t_v.size();
    // Vegas to a good distribution function
    std::vector<int> b(m_n);
    if (t_c < 1)
        throw std::invalid_argument("c < 1, tables with lenght 1 will get less than size 1");
    if (t_k < t_c)
        throw std::invalid_argument("k < c, how do you expect vegas to end?");
    if (t_k/t_c < 2)
        throw std::invalid_argument("(k, c) is too hard for the distribution!");
    while (true) {
        std::fill(b.begin(), b.end(), 0);
        m_hash = UHash<T>();
        for (T &x: t_v) 
            b[m_hash(x) % m_n]++;
        
        u_int64_t sum = 0;
        for (int &b_i: b)
            sum += static_cast<u_int64_t>(m_c*b_i*b_i);
        
        if (sum <= static_cast<u_int64_t>(m_k*m_n)) 
            break;
    }
    
    std::vector<std::vector<T>> B;
    B.reserve(m_n);
    for (auto &b_i: b) {
        B.emplace_back();
        B.back().reserve(b_i);
    }
    
    for (auto &x: t_v)
        B[m_hash(x) % m_n].push_back(x);

    m_table.reserve(m_n);
    m_t_secs.reserve(m_n);
    for (int i = 0; i < m_n; i++) {
        auto [dt, inpht] = delta<innerPerfectHashTable<T>>( 
            [&] () {
            return innerPerfectHashTable<T>(std::ceil(m_c*b[i]*b[i]), B[i]);
            }
        );
        m_table.push_back(std::move(inpht));
        m_t_secs.push_back(dt);
    }
}

template <typename T>
PerfectHashTable<T>::PerfectHashTable(double t_c, double t_k, std::vector<T> &t_v, double p) {
    try {
        auto [dt, _] = delta<int>(
            [&] () {
            this->_PerfectHashTable(t_c, t_k, t_v, p); 
            return 0;
            }
        );
        m_t_tot = dt;
    }
    catch (const std::invalid_argument& e){
        m_fail = true;
    }
}

template <typename T>
bool PerfectHashTable<T>::search(const T &x) {
    return m_table[m_hash(x) % m_n].search(x);
}

/*
* n, c, k, t tot, Δt prim, Δt sec, µt sec, σt sec, m tot, Δm prim, Δm sec, µm sec, σm sec 
* t in µs
* m in B
*/
template <typename T>
std::string PerfectHashTable<T>::benchmark() {
    // Specific case
    std::string ret = "";
    ret += std::to_string(m_n) + ","       // n
        + std::to_string(m_c) + ","        // c
        + std::to_string(m_k) + ",";       // k
    
    // Time
    auto dt_sec = m_fail ? std::nan("") : std::accumulate(m_t_secs.begin(), m_t_secs.end(), 0.0);
    auto dt_prim = m_t_tot - dt_sec;
    auto [mean_t, stddev_t] = stat_mean_dev<int64_t>(m_t_secs);
    ret += m_fail ? std::to_string(std::nan("")) + ",": 
        std::to_string(m_t_tot) + ",";     // t tot
    ret += std::to_string(dt_prim) + ","   // Δt prim
        + std::to_string(dt_sec) + ","     // Δt sec
        + std::to_string(mean_t) + ","     // µt sec
        + std::to_string(stddev_t) + ",";  // σt sec

    // Space
    auto dm_prim = m_fail ? std::nan("") : m_n * sizeof(innerPerfectHashTable<T>);
    std::vector<u_int16_t> m_secs;
    m_secs.reserve(m_n); 
    for (auto &ht: m_table)
        m_secs.push_back(ht.m_n * sizeof(T)); // vec size * sizeof(T)
    auto dm_sec = m_fail ? std::nan("") : std::accumulate(m_secs.begin(), m_secs.end(), 0U) * sizeof(T);
    auto m_tot = dm_prim + dm_sec;
    auto [mean_m, stddev_m] = stat_mean_dev<u_int16_t>(m_secs);
    ret += std::to_string(m_tot) + ","     // m tot
        + std::to_string(dm_prim) + ","    // Δm prim
        + std::to_string(dm_sec) + ","     // Δm sec
        + std::to_string(mean_m) + ","     // µm sec
        + std::to_string(stddev_m);         // σm sec
    return ret;
}