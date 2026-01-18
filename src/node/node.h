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

class Task;

class Node {
public:
    int id_;
    std::shared_ptr<System::System> system_;
    std::vector<IO::Envelope> inbox;
    virtual void dispatch() = 0;
    Node(int id, std::shared_ptr<System::System> system): id_(id), system_(system) {}
    virtual Task main_loop() = 0;
};

// todo: get rid of suspendtoscheduler awaitable, make it suspend_always and give the system a
// enqueue_work method, so we push to the executor a lambda to resume the coroutine
class Task {
public:
    struct promise_type {
        std::shared_ptr<System::System> sys_;
        Task get_return_object() {return Task{std::coroutine_handle<promise_type>::from_promise(*this)};}
        // this intercepts the same arguments with which the coro is called
        // Since I am returning the coro from within the node, the compiler automagically
        // passes a Node& reference to the promise_type constructor.
        promise_type(Node& node) : sys_(node.system_) {}
        std::suspend_always initial_suspend() {return {};}
        std::suspend_never final_suspend() noexcept {return {};}
        void return_void() {}
        void unhandled_exception() {std::terminate();}
    };
    std::coroutine_handle<promise_type> h_;

    // Constructor called by get_return_object()
    explicit Task(std::coroutine_handle<promise_type> handle) : h_(handle) {}

};

class SleeperNode : public Node {
public:
    SleeperNode(int id, std::shared_ptr<System::System> system) : Node(id, system) {}
    Task main_loop() override;
    void dispatch() override;
};

} // namespace Node


#endif // _NODE_H_