#include "src/system/system.h"
#include "src/io/messages.h"
#include "src/io/network.h"
#include "src/scheduler/scheduler.h"
#include "src/executor/executor.h"
#include "src/rng/rng.h"
#include "src/clock/clock.h"
#include "gtest/gtest.h"
#include <coroutine>
#include <memory>
#include <variant>

template<typename T>
class Task {
public:
    struct promise_type {
        T result;
        Task get_return_object() { 
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; 
        }
        bool await_ready() {return false;}
        std::suspend_always initial_suspend() {return {};}
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(T retval) {result = std::move(retval);};
        void unhandled_exception() {std::terminate();}
    };
    std::coroutine_handle<promise_type> h;
    explicit Task(std::coroutine_handle<promise_type> handle) : h(handle) {}
    ~Task() { if (h) h.destroy(); }
};

Task<IO::Envelope> publish_to_system(std::shared_ptr<System::System> sys) {
    int from = 0;
    int to = 1;
    IO::MessageName name = IO::MessageName::PING_REQUEST;
    IO::Message content = IO::PingRequest{};
    IO::Envelope response = co_await sys->rpc(from, to, name, content);
    co_return response;
}

TEST(SystemTest, RPCPublishesToNetworkAndResumesCaller) {

    int seed = 400;
    int max_delay = 0;
    
    auto rng = std::make_shared<RNG::UniformDistributionRange>(seed);
    auto clock = std::make_shared<Clock::DeterministicClock>();
    auto network = std::make_shared<IO::Network>(clock, rng, max_delay);
    auto executor = std::make_shared<Executor::PriorityQueueExecutor>(clock);
    auto scheduler = std::make_shared<Scheduler::DeterministicScheduler>(executor, rng, clock, max_delay);
    auto system = std::make_shared<System::System>(scheduler, clock, rng, network);

    auto caller_coro = publish_to_system(system);
    caller_coro.h.resume();

    auto result = IO::PingResponse{};
    auto response = IO::Envelope{
        0,
        IO::MessageName::PING_RESPONSE,
        1,
        0,
        result
    };
    system->register_rpc_completion(0, response);
    while(executor->has_work()) {
        clock->tick();
        executor->run_until_blocked();
    }
    ASSERT_TRUE(caller_coro.h.done());
    ASSERT_EQ(response, caller_coro.h.promise().result);
    auto expected_envelope = IO::Envelope{
        .message_id = 0,
        .name = IO::MessageName::PING_REQUEST,
        .from = 0,
        .to = 1,
        .content = IO::PingRequest{}
    };
    auto network_items = network->fetch_ready();
    ASSERT_EQ(network_items.size(), 1);
    ASSERT_EQ(network_items[0], expected_envelope);
}