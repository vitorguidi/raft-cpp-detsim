#include "src/io/network.h"
#include "src/node/node.h"
#include "src/routing/router.h"
#include "src/rng/rng.h"
#include "src/clock/clock.h"
#include "src/system/system.h"

#include "gtest/gtest.h"
#include <ranges>
#include <stdexcept>

namespace Routing {

class DummyNode : public Node::Node {
public:
    bool saw_messages_;
    DummyNode(int id, std::shared_ptr<System::System> sys) : Node::Node(id, sys), saw_messages_(false) {}
    void dispatch() {
        saw_messages_ = true;
    }
    ::Node::Task main_loop() {throw std::runtime_error("unimplemented.");}
};

TEST(RouterTest, RouterSendsMessagesToInboxAndSchedulesDispatch) {
    int seed = 400;
    auto clock = std::make_shared<Clock::DeterministicClock>();
    auto rng = std::make_shared<RNG::UniformDistributionRange>(seed);
    auto executor = std::make_shared<Executor::PriorityQueueExecutor>(clock);
    auto sched = std::make_shared<Scheduler::DeterministicScheduler>(executor, rng, clock, 0);
    auto net = std::make_shared<IO::Network>(clock, rng, 0);
    auto sys = std::make_shared<System::System>(sched, clock, rng, net);

    auto node0 = std::make_shared<DummyNode>(0, sys);
    auto node1 = std::make_shared<DummyNode>(1, sys);
    std::vector<std::shared_ptr<Node::Node>> nodes = {node0, node1};

    auto router = std::make_shared<Router>(nodes, net, sys);

    auto msg1 = IO::Envelope{
        .message_id = 0,
        .name = IO::MessageName::PING_REQUEST,
        .from = 0,
        .to = 1,
        .content = IO::PingRequest{},
    };
    net->push_entry(msg1);

    auto msg2 = IO::Envelope{
        .message_id = 1,
        .name = IO::MessageName::PING_RESPONSE,
        .from = 1,
        .to = 0,
        .content = IO::PingResponse{},
    };
    net->push_entry(msg2);

    router->route();

    while(executor->has_work()) {
        clock->tick();
        executor->run_until_blocked();
    }

    ASSERT_EQ(node0->inbox.size(), 1);
    ASSERT_EQ(node0->inbox[0], msg2);
    ASSERT_TRUE(node0->saw_messages_);


    ASSERT_EQ(node1->inbox.size(), 1);
    ASSERT_EQ(node1->inbox[0], msg1);
    ASSERT_TRUE(node1->saw_messages_);
}

} // namespace Router