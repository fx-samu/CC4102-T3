#include "ThreadPool.h"

ThreadPool& ThreadPool::getInstance(size_t num_threads) {
    static ThreadPool instance(num_threads);
    return instance;
}

ThreadPool::ThreadPool(size_t num_threads) : stop(false) {
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(&ThreadPool::workerThread, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &thread : threads) {
        thread.join();
    }
}

void ThreadPool::enqueueTask(int priority, std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(priority, task);
    }
    condition.notify_one();
}

void ThreadPool::workerThread() {
    while (true) {
        Task currentTask(0, [](){});
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty())
                return;
            currentTask = std::move(tasks.top());
            tasks.pop();
        }
        active_tasks++;
        currentTask.func();
        active_tasks--;
    }
}

bool ThreadPool::queueEmpty() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    return tasks.empty();
}

bool ThreadPool::taskFinished() {
    return active_tasks == 0;
}
bool ThreadPool::inactive() {
    return queueEmpty() && taskFinished();
}