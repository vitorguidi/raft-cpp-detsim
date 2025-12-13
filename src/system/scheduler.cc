#include "scheduler.h"
#include <thread>
#include <queue>
#include <atomic>
#include <stdexcept>

void Executor::push_task(std::function<void()> task) {
    std::unique_lock pl(q_mu_);
    tasks_.push(task);
    condition_.notify_one();
}

Executor::Executor(int nr_threads): nr_threads_(nr_threads), done_(false) {
    if (nr_threads == 0){
        throw std::invalid_argument("ThreadPool must have at least one thread.");
    }
    for(int i=0;i<nr_threads;i++) {
        workers_.emplace_back([this](){
            std::function<void()> task;
            while(true) {
                {
                    std::unique_lock wl(this->q_mu_);
                    condition_.wait(wl, [this](){return !this->tasks_.empty() || this->done_;});
                    if (this->tasks_.empty()) {
                        return;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            }
        });
    }
}

Executor::~Executor() {
    {
        std::unique_lock dl(q_mu_);
        done_ = true;
    }
    condition_.notify_all();
    for(auto& worker: workers_) {
        worker.join();
    }
}