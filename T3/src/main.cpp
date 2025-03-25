#include "includes.h"

#define WORKING_THREADS 16
#define VAR_TYPE int

using namespace std;

string dir;
string header = "n, c, k, t_tot, dt_prim, dt_sec, mean_t_sec, dev_t_sec, m_tot, dm_prim, dm_sec, mean_m_sec, dev_m_sec";

mutex log_m;
condition_variable log_c;
queue<pair<int, string>> log_q; // exp, benchmark()

void writer() {
    ThreadPool& tp = ThreadPool::getInstance();
    cout << "E, " << header << endl;
    vector<ofstream> log_v;
    filesystem::create_directory(dir);

    for (int i = 1; i <=4; i++) {
        string name = dir + "/"+ to_string(i) + ".csv";
        log_v.emplace_back(name);
        if (!log_v.back().is_open()) {
            throw runtime_error("Failed to open file: " + name);
        }
        log_v.back().setf(ios::unitbuf);
        log_v.back() << header << "\n";
    }

    bool log_empty = false;
    int e;
    string out;

    do {
        {
            unique_lock<mutex> lock(log_m);
            if (!tp.inactive()) 
                log_c.wait_for(lock, chrono::seconds(30),[&]() {return !tp.inactive();} );

            log_empty = log_q.empty();
            if (!log_empty) {
                auto [p1, p2] = log_q.front();
                e   = p1;
                out = p2;
                log_q.pop();
            }
            else
                continue;
        }
        cout << e << ", " << out << endl;
        log_v[e-1] << out << "\n";
    } while (!(log_empty && tp.inactive()));
    
    for (auto& file : log_v) {
        file.close();
    }
}

template <typename T>
vector<T> randomVecN (uint64_t n) {
    double t_min = numeric_limits<T>::min();
    double t_max = numeric_limits<T>::max();
    if (t_max - t_min + 1 < n)
        throw invalid_argument("The expected size is larger than the type");
    // 2 ^ 16 ~ 10^5 is acceptable, since the procesor goes 10^8 x100 faster
    // if ((n/(t_max - t_min + 1)*100 > 5) && (n > (1<<16) + 1))
    //     throw invalid_argument("Algorithm may never end");
    unordered_set<T> unique_nums;

    random_device rd;
    mt19937_64 gen(rd());

    function<T()> dist;    
    if constexpr (is_floating_point<T>::value) {
        uniform_real_distribution<T> f(t_min, t_max);
        dist = [f, &gen]() mutable { return f(gen); };
    }   
    else {
        uniform_int_distribution<T> f(t_min, t_max);
        dist = [f, &gen]()mutable { return f(gen); };
    }

    while (unique_nums.size() < n) {
        unique_nums.insert(dist());
    }

    return vector<T>(unique_nums.begin(), unique_nums.end());
    return vector<T>();
}

template <typename T>
vector<T> linspace(T start, T end, int num) {
    vector<T> result;
    if (num <= 0) return result; // Si num es 0 o menos, devolvemos un vector vacío
    if (num == 1) {
        result.push_back(start); // Un solo punto, devolvemos solo el inicio
        return result;
    }

    double step = (end - start) / (num - 1);
    if (is_floating_point<T>::value)
        for (int i = 0; i < num; ++i)
            result.push_back(static_cast<T>(start + i * step));
    else 
        for (int i = 0; i < num; ++i)
            result.push_back(static_cast<T>(round(start + i * step)));
    
    return result;
}

template <typename T>
void test(int e, double c, double k, uint64_t n) {
    vector<T> v = randomVecN<T>(n);
    PerfectHashTable<T> pht(c, k, v);
    string out = pht.benchmark();
    {
        unique_lock<mutex> lock(log_m);
        log_q.push({e, out});
    }
    log_c.notify_one();
}

int main() {
    auto now = chrono::system_clock::now();
    auto current_time = chrono::system_clock::to_time_t(now);
    auto local_time = *localtime(&current_time);
    ostringstream time_f;
    time_f << put_time(&local_time, "%Y-%m-%d_%H-%M-%S/");
    dir = "../logs/" + time_f.str();

    // vector<int> v = randomVecN<int>(1e6);
    // PerfectHashTable<int> pht(3, 4, v);
    // cout << pht.benchmark() << endl;

    ThreadPool& tp = ThreadPool::getInstance(WORKING_THREADS);

    // Only for testing pourposes, 
    VAR_TYPE max_range = 1e7 < numeric_limits<VAR_TYPE>::max() ? 1e7 : numeric_limits<VAR_TYPE>::max();

    // Exp 1
    // c = 1, k = 4, n var
    auto n_exp_1 = linspace<VAR_TYPE>(1000, max_range, 100);
    for (auto& n: n_exp_1) {
        tp.enqueueTask(1,  [=](){
            test<VAR_TYPE>(1, 1, 4, n);
        } );
    }

    // Exp 2
    // c = 1, k = var, n = 10^6
    auto vec_n = 1e6 < max_range ? 1e6 : max_range;
    auto k_exp_2 = linspace<double>(1, 8, 100);
    for (auto& k: k_exp_2) {
        tp.enqueueTask(1,  [=](){
            test<VAR_TYPE>(2, 1, k, vec_n);
        } );
    }

    // Exp 3
    // c = var, k = 4, n = 10^6
    auto c_exp_3 = linspace<double>(1, 4, 100);
    for (auto& c: c_exp_3) {
        tp.enqueueTask(1,  [=](){
            test<VAR_TYPE>(3, c, 4, vec_n);
        } );
    }

    // Exp 4
    // c = var, k = var, n = intervals
    // c y k tecnicamente actuan de la misma forma
    n_exp_1 = linspace<VAR_TYPE>(100, max_range, 5);
    k_exp_2 = linspace<double>(1, 8, 25);
    c_exp_3 = linspace<double>(1, 4, 25);
    vec_n = 1e5 < max_range ? 1e6 : max_range;
    
    for (auto &n: n_exp_1)
        for (auto& k: k_exp_2)
            for (auto& c: c_exp_3) 
                tp.enqueueTask(0,  [=](){
                test<VAR_TYPE>(4, c, k, n);
                } );
            
        

    writer();

    return 0;
}