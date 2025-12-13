#ifndef _SYSTEM_SCHEDULER
#define _SYSTEM_SCHEDULER

#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

class Executor {
public:
    void push_task(std::function<void()> task);
    Executor(int nr_threads);
    ~Executor();
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex q_mu_;
    std::condition_variable condition_;
    int nr_threads_;
    bool done_;
};

#endif