#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <functional>
#include "src/clock/clock.h"
#include "src/executor/executor.h"
#include "src/clock/clock.h"
#include "src/rng/rng.h"

namespace Scheduler {

class Scheduler {
public:
    virtual void schedule_task(std::function<void()> task) = 0;
    virtual void schedule_task_with_delay(std::function<void()> task, int delay) = 0;
    virtual ~Scheduler() = default;
};

class DeterministicScheduler : public Scheduler {
private:
    std::shared_ptr<Executor::Executor> executor_;
    std::shared_ptr<RNG::RNG> rng_;
    std::shared_ptr<Clock::Clock> clock_;
    int base_jitter_;
public:
    void schedule_task(std::function<void()> task) override;
    void schedule_task_with_delay(std::function<void()> task, int delay) override;
    DeterministicScheduler(
        std::shared_ptr<Executor::Executor> executor,
        std::shared_ptr<RNG::RNG> rng,
        std::shared_ptr<Clock::Clock> clock,
        int base_jitter
    );
    ~DeterministicScheduler() = default;
};

}

#endif // _SCHEDULER_H_