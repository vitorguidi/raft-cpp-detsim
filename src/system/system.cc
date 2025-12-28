#include "src/system/system.h"
#include "src/scheduler/scheduler.h"
#include <memory>
#include <coroutine>

namespace System {

System::System(
        std::shared_ptr<Executor::Executor> executor,
        std::shared_ptr<Scheduler::Scheduler> scheduler,
        std::shared_ptr<Clock::Clock> clock)
        : executor_(std::move(executor)), scheduler_(std::move(scheduler)), clock_(std::move(clock)) {}


void System::tick() {
    executor_->run_until_blocked();
}

SleepRequest::SleepRequest(int delay, std::shared_ptr<Scheduler::Scheduler> scheduler)
    : delay_(delay), scheduler_(std::move(scheduler)) {}
bool SleepRequest::await_ready() {return false;}
void SleepRequest::await_suspend(std::coroutine_handle<> h) const {
    auto resumer_lambda = [h]() {
        h.resume();
    };
    scheduler_->schedule_task_with_delay(std::move(resumer_lambda), delay_);
}
void SleepRequest::await_resume() const noexcept {}
} // namespace System