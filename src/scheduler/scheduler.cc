#include "src/scheduler/scheduler.h"
#include "src/executor/executor.h"
#include "src/rng/rng.h"
#include <memory>
#include <iostream>
#include <thread>

namespace Scheduler {

DeterministicScheduler::DeterministicScheduler(
    std::shared_ptr<Executor::Executor> executor,
    std::shared_ptr<RNG::RNG> rng,
    std::shared_ptr<Clock::Clock> clock,
    int base_jitter)
        : executor_(std::move(executor)), rng_(std::move(rng)), clock_(std::move(clock)), base_jitter_(base_jitter) {}

void DeterministicScheduler::schedule_task(std::function<void()> task) {
    int jitter = rng_->draw(0, base_jitter_);
    int now = clock_->now();
    executor_->push_task(std::move(task), now + jitter);
    std::cout << "[Scheduler] time = " << now << ", scheduled for time " << now + jitter << std::endl;
}        

void DeterministicScheduler::schedule_task_with_delay(std::function<void()> task, int delay) {
    int now = clock_->now();
    int jitter = rng_->draw(0, base_jitter_);
    executor_->push_task(std::move(task), now + delay + jitter);
    std::cout << "[Scheduler]  time = " << now << ", scheduled for time " << now + delay + jitter << std::endl;
}

}
