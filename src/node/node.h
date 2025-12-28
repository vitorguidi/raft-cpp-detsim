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

class Task {
public:
    struct promise_type {
        std::shared_ptr<Scheduler::Scheduler> sched_;
        Task get_return_object() {return {};}
        // this intercepts the same arguments with which the coro is called
        promise_type(std::shared_ptr<Scheduler::Scheduler> sched, int delay) 
            : sched_(sched) {}
        auto initial_suspend() {
            struct SuspendToScheduler {
                std::shared_ptr<Scheduler::Scheduler> sched_;
                bool await_ready() {return false;}
                void await_resume() {}
                void await_suspend(std::coroutine_handle<> h) {
                    sched_->schedule_task([h](){h.resume();});
                }
            };
            return SuspendToScheduler{sched_};
        }
        std::suspend_never final_suspend() noexcept {return {};}
        void return_void() {}
        void unhandled_exception() {std::terminate();}
    };
};

Task NodeMainLoop(std::shared_ptr<Scheduler::Scheduler> sched_, int delay) {
    std::cout << "[Looper thread] Starting looper" << std::endl;
    for(int i=0;i<10;i++) {
        std::cout << "[Looper thread]Sleeping before co await" << std::endl;
        co_await System::SleepRequest(delay, sched_);
        std::cout << "[Looper thread]Awoke after co await returned" << std::endl;
    }
    std::cout << "[Looper thread] Finished looper" << std::endl;
}


} // namespace Node


#endif // _NODE_H_