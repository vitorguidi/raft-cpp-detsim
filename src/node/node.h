#ifndef _NODE_H_
#define _NODE_H_

#include <functional>
#include <coroutine>
#include <exception>
#include <memory>
#include "src/system/system.h"
#include <iostream>

namespace Node {

class Task {
private:
    std::shared_ptr<Scheduler::Scheduler> sched_;
public:
    struct promise_type {
        Task get_return_object() {return {};}
        std::suspend_always initial_suspend() {return {};}
        std::suspend_never final_suspend() noexcept {return {};}
        void return_void() {}
        void unhandled_exception() {std::terminate();}
    };
};

class Node {
public:
    virtual std::function<void()> main_loop() = 0;
};


Task NodeMainLoop(std::shared_ptr<Scheduler::Scheduler> sched_, int delay) {
    for(int i=0;i<10;i++) {
        std::cout << "Sleeping before co await" << std::endl;
        co_await System::SleepRequest(delay, sched_);
        std::cout << "Awoke after co await returned" << std::endl;
    }
}


} // namespace Node


#endif // _NODE_H_