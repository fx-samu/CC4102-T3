#pragma once

#include "includes.h"

struct Task {
    int priority;
    std::function<void()> func;

    // Constructor
    Task(int p, std::function<void()> f) : priority(p), func(f) {}

    // Operador de comparación para la cola de prioridad
    bool operator<(const Task& other) const {
        // Invertimos la comparación porque priority_queue es un max-heap
        return priority < other.priority;
    }
};

// Implementación del ThreadPool con cola de prioridad
class ThreadPool {
public:
    // Obtener la instancia Singleton del ThreadPool
    static ThreadPool& getInstance(size_t num_threads = std::thread::hardware_concurrency());

    // Encolar tarea con prioridad
    void enqueueTask(int priority, std::function<void()> task);

    // Sin tareas en cola
    bool queueEmpty();

    // Sin tareas en ejecucion
    bool taskFinished();

    // Sin tareas en cola ni ejecucion
    bool inactive();

    // Destructor
    ~ThreadPool();

private:
    // Constructor privado para Singleton
    ThreadPool(size_t num_threads);

    // Función que ejecutan los hilos
    void workerThread();

    std::priority_queue<Task> tasks;
    std::mutex queue_mutex;
    std::vector<std::thread> threads;
    std::condition_variable condition;
    bool stop;
    std::atomic<int> active_tasks = 0;
};

#include "../src/ThreadPool.tpp"