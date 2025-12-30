#include "src/rng/rng.h"
#include "src/system/system.h"
#include "src/executor/executor.h"
#include "memory"
#include <thread>

int main() {
    int seed = 400;
    auto rng = std::make_shared<RNG::UniformDistributionRange>(seed);
    auto clock = std::make_shared<Clock::DeterministicClock>();
    auto executor = std::make_shared<Executor::PriorityQueueExecutor>(clock);
    auto scheduler = std::make_shared<Scheduler::DeterministicScheduler>(executor, rng, clock, 100);
    auto system = std::make_shared<System::System>(executor, scheduler, clock, rng);

    std::cout << "Simulation starting" << std::endl;

    auto single_node = std::make_shared<System::SleeperNode>(0, system);
    system->add_node(single_node);
    auto coro = single_node->main_loop();
    for(int i=0;i<10000;i++) {
        system->tick();
        clock->tick();
    }

    std::cout << "Simulation finished" << std::endl;

}
