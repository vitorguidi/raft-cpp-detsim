#include "src/rng/rng.h"
#include "src/node/node.h"
#include "src/executor/executor.h"
#include "src/routing/router.h"
#include "memory"
#include <thread>

int main() {
    int seed = 400;
    const int MAX_NETWORK_DELAY = 100;
    const int MAX_TASK_SCHEDULE_JITTER = 100;
    auto rng = std::make_shared<RNG::UniformDistributionRange>(seed);
    auto clock = std::make_shared<Clock::DeterministicClock>();
    auto executor = std::make_shared<Executor::PriorityQueueExecutor>(clock);
    auto scheduler = std::make_shared<Scheduler::DeterministicScheduler>(executor, rng, clock, MAX_TASK_SCHEDULE_JITTER);
    auto network = std::make_shared<IO::Network>(clock, rng, MAX_NETWORK_DELAY);
    auto system = std::make_shared<System::System>(scheduler, clock, rng, network);
    
    const int NR_NODES=5;
    std::vector<std::shared_ptr<Node::Node>> nodes;

    for(int i=0;i<5;i++) {
        auto node = std::make_shared<Node::PingerNode>(i, system, NR_NODES);
        nodes.push_back(node);
        auto main_loop = node->main_loop();
        auto starter_lambda = [main_loop]() {main_loop.h_.resume();};
        system->request_work(starter_lambda);
    }
    
    auto router = std::make_shared<Routing::Router>(nodes, network, system);

    std::cout << "Simulation starting" << std::endl;

    while(executor->has_work() || network->has_messages()) {
        clock->tick();
        router->route();
        executor->run_until_blocked();
    }

    std::cout << "Simulation finished" << std::endl;

}
