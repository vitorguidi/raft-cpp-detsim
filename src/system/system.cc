#include "src/system/system.h"
#include "src/scheduler/scheduler.h"
#include <memory>
#include <coroutine>
#include <iostream>
#include <vector>

namespace System {

// System stuff
System::System(
        std::shared_ptr<Scheduler::Scheduler> scheduler,
        std::shared_ptr<Clock::Clock> clock,
        std::shared_ptr<RNG::RNG> rng)
        :   scheduler_(std::move(scheduler)),
            clock_(std::move(clock)),
            rng_(std::move(rng)) {}

void System::add_node(std::shared_ptr<Node> node) {nodes_.push_back(node);}

long long int System::get_time() {return clock_->now();}
int System::random_range(int lo, int hi) {return rng_->draw(lo, hi);}
System::SleepRequest System::sleep(int delay) {return System::SleepRequest(delay, scheduler_);}


// SleepRequest stuff
System::SleepRequest::SleepRequest(int delay, std::shared_ptr<Scheduler::Scheduler> sched) : delay_(delay), sched_(sched) {}
bool System::SleepRequest::await_ready() {return false;}
void System::SleepRequest::await_suspend(std::coroutine_handle<> h) const {
    auto resumer_lambda = [h]() {
        h.resume();
    };
    sched_->schedule_task_with_delay(std::move(resumer_lambda), delay_);
}
void System::SleepRequest::await_resume() const noexcept {}
} // namespace System