#ifndef _NODE_H_
#define _NODE_H_

#include <functional>
#include <coroutine>
#include <exception>
#include <memory>
#include "src/system/system.h"
#include <iostream>
#include <thread>

namespace Node {

class LoopTask;

class Node {
friend class SleeperNode;
public:
    int id_;
    std::shared_ptr<System::System> system_;
    Node(int id, std::shared_ptr<System::System> system): id_(id), system_(system) {}
    virtual LoopTask main_loop() = 0;
};

class SleeperNode : public Node {
public:
    SleeperNode(int id, std::shared_ptr<System::System> system) : Node(id, system) {}
    LoopTask main_loop() override;
};

class LoopTask {
public:
    struct promise_type {
        std::shared_ptr<System::System> sys_;
        LoopTask get_return_object() {return {};}
        // this intercepts the same arguments with which the coro is called
        // Since I am returning the coro from within the node, the compiler automagically
        // passes a Node& reference to the promise_type constructor.
        promise_type(Node& node) : sys_(node.system_) {}
        auto initial_suspend() {
            struct SuspendToScheduler {
                std::shared_ptr<System::System> sys_;
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

} // namespace Node


#endif // _NODE_H_