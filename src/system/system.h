#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "src/scheduler/scheduler.h"
#include "src/rng/rng.h"
#include "src/executor/executor.h"
#include <coroutine>
#include <memory>
#include <functional>
#include <exception>
#include <iostream>

namespace System {

class System;
class Node;

class System {
public:
    std::shared_ptr<Executor::Executor> executor_;
    std::shared_ptr<Scheduler::Scheduler> scheduler_;
    std::shared_ptr<Clock::Clock> clock_;
    std::shared_ptr<RNG::RNG> rng_;
    std::vector<std::shared_ptr<Node>> nodes_;
    System(
        std::shared_ptr<Executor::Executor> executor,
        std::shared_ptr<Scheduler::Scheduler> scheduler,
        std::shared_ptr<Clock::Clock> clock,
        std::shared_ptr<RNG::RNG> rng);
    void add_node(std::shared_ptr<Node> node);
    int random_range(int lo, int hi);
    long long int get_time();
    void tick();
    class SleepRequest;
    SleepRequest sleep(int delay);
    template<typename T, typename U>
    class RPC;
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

class LoopTask;

class Node {
friend class SleeperNode;
public:
    int id_;
    std::shared_ptr<System> system_;
    Node(int id, std::shared_ptr<System> system): id_(id), system_(system) {}
    virtual LoopTask main_loop() = 0;
};

class SleeperNode : public Node {
public:
    SleeperNode(int id, std::shared_ptr<System> system) : Node(id, system) {}
    LoopTask main_loop() override;
};

class LoopTask {
public:
    struct promise_type {
        std::shared_ptr<System> sys_;
        LoopTask get_return_object() {return {};}
        // this intercepts the same arguments with which the coro is called
        // Since I am returning the coro from within the node, the compiler automagically
        // passes a Node& reference to the promise_type constructor.
        promise_type(Node& node) : sys_(node.system_) {}
        auto initial_suspend() {
            struct SuspendToScheduler {
                std::shared_ptr<System> sys_;
                bool await_ready() {return false;}
                void await_resume() {}
                void await_suspend(std::coroutine_handle<> h) {
                    sys_->scheduler_->schedule_task([h](){h.resume();});
                }
            };
            return SuspendToScheduler{sys_};
        }
        std::suspend_never final_suspend() noexcept {return {};}
        void return_void() {}
        void unhandled_exception() {std::terminate();}
    };
};


} // namespace System
#endif //