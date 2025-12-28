#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "src/scheduler/scheduler.h"
#include "src/executor/executor.h"
#include <coroutine>
#include <memory>


namespace System {

class System {
private:
    std::shared_ptr<Executor::Executor> executor_;
    std::shared_ptr<Scheduler::Scheduler> scheduler_;
    std::shared_ptr<Clock::Clock> clock_;
public:
    System(
        std::shared_ptr<Executor::Executor> executor,
        std::shared_ptr<Scheduler::Scheduler> scheduler,
        std::shared_ptr<Clock::Clock> clock);
    void tick();
};

class SleepRequest {
private:
    int delay_;
    std::shared_ptr<Scheduler::Scheduler> scheduler_;
public:
    SleepRequest(int delay, std::shared_ptr<Scheduler::Scheduler> sched);
    bool await_ready();
    void await_suspend(std::coroutine_handle<> h) const;
    void await_resume() const noexcept;
};

} // namespace System
#endif //