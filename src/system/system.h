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
#include <optional>

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
class RPCTask;

struct PingRequest {
    int from;
};

struct PingResponse {
    int from;
};

class Node {
friend class SleeperNode;
public:
    int id_;
    std::shared_ptr<System> system_;
    Node(int id, std::shared_ptr<System> system): id_(id), system_(system) {}
    virtual LoopTask main_loop() = 0;
    virtual RPCTask handle_rpc(PingRequest request) = 0;
};



class RPCTask {
public:
    struct promise_type {
        std::shared_ptr<System> sys_;
        PingResponse response_;
        std::coroutine_handle<> waiter_;


        RPCTask get_return_object() {
            return RPCTask{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        // this intercepts the same arguments with which the coro is called
        // Since I am returning the coro from within the node, the compiler automagically
        // passes a Node& reference to the promise_type constructor.
        promise_type(Node& node, auto&& ...) : sys_(node.system_) {}


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
        struct FinalAwaitable {
            bool await_ready() noexcept { return false; }
            void await_resume() noexcept {}
            std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> h) noexcept {
                if (h.promise().waiter_) return h.promise().waiter_;
                throw std::runtime_error("Was supposed to have the caller register an rpc callback");
            }
        };
        auto final_suspend() noexcept { return FinalAwaitable{}; }
        void return_value(PingResponse result) {
            response_ = std::move(result);
        }
        void unhandled_exception() {std::terminate();}
    };
    std::coroutine_handle<promise_type> handle_;
    explicit RPCTask(std::coroutine_handle<promise_type> h) : handle_(h) {}
    promise_type& promise() { return handle_.promise(); }
   

};

class PingerNode : public Node {
public:
    PingerNode(int id, std::shared_ptr<System> system) : Node(id, system) {}
    ~PingerNode() = default;
    LoopTask main_loop() override;
    RPCTask handle_rpc(PingRequest request) override;
};

class System::RPC {
private:
    int from_, to_;
    PingRequest request_payload_;
    std::shared_ptr<System> system_;
    std::optional<RPCTask> handler_;
public:
    RPC(int from, int to, PingRequest request, std::shared_ptr<System> system) :
        from_(from), to_(to), request_payload_(request), system_(system) {}
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        auto task = system_->nodes_[to_]->handle_rpc(request_payload_);
        task.promise().waiter_ = h;
        handler_ = (std::move(task));
    }
    PingResponse await_resume() {
    return std::move(handler_->promise().response_);
    }
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