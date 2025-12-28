#ifndef _EXECUTOR_H_
#define _EXECUTOR_H_

#include "src/clock/clock.h"
#include <functional>
#include <memory>
#include <tuple>
#include <queue>

namespace Executor {

struct PendingTask {
public:
    long long int time, id;
    std::function<void()> task;
    bool operator<(const PendingTask& other) const {
        return std::make_tuple(time, id) > std::make_tuple(other.time, other.id);
    }
};

class Executor {
public:
    virtual ~Executor() = default;
    virtual void push_task(std::function<void()> task, long long int time) = 0;
    virtual void run_until_blocked() = 0;
};

class PriorityQueueExecutor : public Executor {
public:
    void push_task(std::function<void()> task, long long int time) override;
    void run_until_blocked() override;
    PriorityQueueExecutor(std::shared_ptr<Clock::Clock> clk);
    ~PriorityQueueExecutor() = default;
private:
    std::priority_queue<PendingTask> tasks_;
    std::shared_ptr<Clock::Clock> clock_;
    long long int task_counter_;
};

} // namespace Executor

#endif // _EXECUTOR_H_