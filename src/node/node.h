#ifndef _NODE_H_
#define _NODE_H_

#include <functional>
#include <coroutine>
#include <exception>
#include <memory>
#include "src/system/system.h"
#include <iostream>
#include <thread>
#include <optional>

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

        template<typename... Args>
        promise_type(Node& node, Args&&...) : sys_(node.system_) {}

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

class PingerNode : public Node {
private:
    int nr_nodes_;
    std::vector<Task> active_handlers_;
public:
    PingerNode(int id, std::shared_ptr<System::System> system, int nr_nodes) :  Node(id, system), nr_nodes_(nr_nodes) {}
    void dispatch() override;
    Task handle_ping(IO::Envelope msg);
    Task main_loop() override;
};

enum RaftState {
    LEADER,
    FOLLOWER,
    CANDIDATE,
};

class RaftNode : public Node {
private:
    RaftState state_ = FOLLOWER;
    int term_ = 0;
    int nr_nodes_;
    std::optional<int> voted_for_;
    int votes_received_ = 0;
    long long last_heartbeat_time_ = 0;
    int election_timeout_min_;
    int election_timeout_max_;
    int heartbeat_interval_;
public:
    RaftNode(int id, std::shared_ptr<System::System> sys, int nr_nodes,
             int election_timeout_min, int election_timeout_max, int heartbeat_interval)
        : Node(id, sys), nr_nodes_(nr_nodes),
          election_timeout_min_(election_timeout_min),
          election_timeout_max_(election_timeout_max),
          heartbeat_interval_(heartbeat_interval) {}
    void dispatch() override;
    Task main_loop() override;
    Task handle_append_entries(IO::Envelope msg);
    Task handle_request_vote(IO::Envelope msg);

    // Getters for oracle to inspect state
    RaftState get_state() const { return state_; }
    int get_term() const { return term_; }
};

} // namespace Node


#endif // _NODE_H_