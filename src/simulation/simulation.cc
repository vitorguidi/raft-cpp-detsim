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

    std::vector<System::LoopTask> coros;

    for(int i=0;i<5;i++) {
        auto pinger_node = std::make_shared<System::PingerNode>(i, system);
        system->add_node(pinger_node);
        coros.push_back(pinger_node->main_loop());
    }

    // expecting 30 iterations * 5 nodes = 150 pings, 150 pongs

    while(system->executor_->has_work()) {
        system->tick();
        clock->tick();
    }

    std::cout << "Simulation finished" << std::endl;

}
