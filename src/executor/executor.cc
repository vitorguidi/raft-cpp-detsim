#include "src/executor/executor.h"
#include <thread>
#include <queue>
#include <atomic>
#include <stdexcept>
#include <iostream>

namespace Executor {

void PriorityQueueExecutor::push_task(std::function<void()> task, long long int time) {
    tasks_.push(PendingTask{
        .time = time,
        .id = task_counter_++,
        .task = task
    });
}

PriorityQueueExecutor::PriorityQueueExecutor(std::shared_ptr<Clock::Clock> clk)
    :   clock_(std::move(clk)), task_counter_(0) {}

bool PriorityQueueExecutor::has_work() {return !tasks_.empty();}

void PriorityQueueExecutor::run_until_blocked() {
    while(true) {
        if (tasks_.empty()) {
            break;
        }
        auto front_task = tasks_.top();
        if (front_task.time > clock_->now()) {
            break;
        }
        std::cout << "[Executor] Pulled task at instant " << clock_->now() << std::endl;
        tasks_.pop();
        front_task.task();
    }
}

} // namespace Executor