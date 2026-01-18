#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "src/scheduler/scheduler.h"
#include "src/rng/rng.h"
#include "src/executor/executor.h"
#include "src/io/messages.h"
#include "src/io/network.h"
#include "unordered_map"
#include <coroutine>
#include <memory>
#include <iostream>


namespace System {

class System {
public:
    std::shared_ptr<Scheduler::Scheduler> scheduler_;
    std::shared_ptr<Clock::Clock> clock_;
    std::shared_ptr<RNG::RNG> rng_;
    int message_id_;
    std::shared_ptr<IO::Network> network_;
    std::unordered_map<int, std::coroutine_handle<>> suspended_rpcs;
    std::unordered_map<int, IO::Envelope> responses;
    System(
        std::shared_ptr<Scheduler::Scheduler> scheduler,
        std::shared_ptr<Clock::Clock> clock,
        std::shared_ptr<RNG::RNG> rng,
        std::shared_ptr<IO::Network> network)
        : scheduler_(std::move(scheduler)),
          clock_(std::move(clock)),
          rng_(rng),
          message_id_(0),
          network_(network) {}
    void request_work(std::function<void()> task) {
        scheduler_->schedule_task(task);
    }
    void send_message(IO::Envelope msg) {
        network_->push_entry(msg);
    }
    int random_range(int lo, int hi) {return rng_->draw(lo, hi);}
    long long int get_time() {return clock_->now();}
    int register_pending_rpc(std::coroutine_handle<> h, int from, int to, IO::MessageName name, IO::Message msg) {
        int id = message_id_++;
        suspended_rpcs[id] = h;
        network_->push_entry(IO::Envelope{
            id,
            name,
            from,
            to,
            msg
        });
        return id;
    }
    void register_rpc_completion(int message_id, IO::Envelope response) {
        if (!suspended_rpcs.count(message_id)) {
            throw std::runtime_error("RPC not registered for id " + std::to_string(message_id));
        }
        responses[message_id] = response;
        auto waiter = suspended_rpcs[message_id];
        scheduler_->schedule_task([waiter](){waiter.resume();});
    }
    IO::Envelope get_response(int message_id) {
        if (!responses.count(message_id)) {
            throw std::runtime_error("Response not registered for id " + std::to_string(message_id));
        }
        auto result_it = responses.find(message_id);
        auto result = *result_it;
        responses.erase(result_it);
        return result.second;
    }
    class SleepRequest;
    SleepRequest sleep(int delay);
    class RPC;
    RPC rpc(int from, int to, IO::MessageName rpc_name, IO::Message msg);
};

class System::SleepRequest {
private:
    int delay_;
    std::shared_ptr<Scheduler::Scheduler> sched_;
public:
    SleepRequest(System& sys, int delay) : delay_(delay), sched_(sys.scheduler_) {}
    bool await_ready() {return false;}
    void await_suspend(std::coroutine_handle<> h) const {
        auto resumer_lambda = [h]() {
            h.resume();
        };
        sched_->schedule_task_with_delay(std::move(resumer_lambda), delay_);
    }
    void await_resume() const noexcept {}
};

inline System::SleepRequest System::sleep(int delay) {
    return SleepRequest(*this, delay);
}

class System::RPC {
private:
    IO::Envelope response_;
    System* sys_;
    int message_id_, from_, to_;
    IO::MessageName msg_name_;
    IO::Message request_;
public:
    RPC(System& sys, int from, int to, IO::MessageName rpc_name, IO::Message msg)
          : sys_(&sys),
            message_id_(-1),
            from_(from),
            to_(to),
            msg_name_(rpc_name),
            request_(msg) {}
    bool await_ready() {return false;}
    void await_suspend(std::coroutine_handle<> caller_handle) {
        message_id_ =  sys_->register_pending_rpc(caller_handle, from_, to_, msg_name_, request_);
    }
    IO::Envelope await_resume() {
        if (message_id_ == -1) {
            throw std::runtime_error("Expected message id to be set.");
        }
        return sys_->get_response(message_id_);
    }
};

inline System::RPC System::rpc(int from, int to, IO::MessageName rpc_name, IO::Message msg) {
    return System::RPC(*this, from, to, rpc_name, std::move(msg));
}

} // namespace System
#endif // _SYSTEM_H_