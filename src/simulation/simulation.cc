#include "src/rng/rng.h"
#include "src/node/node.h"
#include "src/executor/executor.h"
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

    std::cout << "Simulation starting" << std::endl;

    Node::SleeperNode single_node(0, system);
    auto coro = single_node.main_loop();
    system->request_work([coro](){coro.h_.resume();});
    for(int i=0;i<10000;i++) {
        clock->tick();
        executor->run_until_blocked();
    }

    std::cout << "Simulation finished" << std::endl;

}
