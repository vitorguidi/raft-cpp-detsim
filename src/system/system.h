#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "src/scheduler/scheduler.h"
#include "src/rng/rng.h"
#include "src/executor/executor.h"
#include <coroutine>
#include <memory>


namespace System {

class System {
public:
    std::shared_ptr<Scheduler::Scheduler> scheduler_;
    std::shared_ptr<Clock::Clock> clock_;
    std::shared_ptr<RNG::RNG> rng_;
    System(
        std::shared_ptr<Scheduler::Scheduler> scheduler,
        std::shared_ptr<Clock::Clock> clock,
        std::shared_ptr<RNG::RNG> rng)
        : scheduler_(std::move(scheduler)),
          clock_(std::move(clock)),
          rng_(rng) {}
    int random_range(int lo, int hi) {return rng_->draw(lo, hi);}
    long long int get_time() {return clock_->now();}
    class SleepRequest;
    SleepRequest sleep(int delay);
};

class System::SleepRequest {
private:
    int delay_;
    std::shared_ptr<Scheduler::Scheduler> sched_;
public:
    SleepRequest(System& sys, int delay) : delay_(delay), sched_(sys.scheduler_) {}
    bool await_ready() {return false;}
    void await_suspend(std::coroutine_handle<> h) const {
        auto resumer_lambda = [h]() {
            h.resume();
        };
        sched_->schedule_task_with_delay(std::move(resumer_lambda), delay_);
    }
    void await_resume() const noexcept {}
};

inline System::SleepRequest System::sleep(int delay) {
    return SleepRequest(*this, delay);
}

} // namespace System
#endif //