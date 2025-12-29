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
    std::shared_ptr<Executor::Executor> executor_;
    std::shared_ptr<Scheduler::Scheduler> scheduler_;
    std::shared_ptr<Clock::Clock> clock_;
    std::shared_ptr<RNG::RNG> rng_;
    System(
        std::shared_ptr<Executor::Executor> executor,
        std::shared_ptr<Scheduler::Scheduler> scheduler,
        std::shared_ptr<Clock::Clock> clock,
        std::shared_ptr<RNG::RNG> rng);
    int random_range(int lo, int hi);
    long long int get_time();
    void tick();
    class SleepRequest;
    SleepRequest sleep(int delay);
};

class System::SleepRequest {
    private:
        int delay_;
        std::shared_ptr<Scheduler::Scheduler> sched_;
    public:
        SleepRequest(int delay, std::shared_ptr<Scheduler::Scheduler> sched_);
        bool await_ready();
        void await_suspend(std::coroutine_handle<> h) const;
        void await_resume() const noexcept;
};

} // namespace System
#endif //