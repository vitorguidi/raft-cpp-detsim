#include "src/rng/rng.h"
#include "src/node/node.h"
#include "src/executor/executor.h"
#include "memory"
#include <thread>

int main() {
    int seed = 400;
    auto rng = std::make_shared<RNG::UniformDistributionRange>(seed, 0, 100);
    auto clock = std::make_shared<Clock::DeterministicClock>();
    auto executor = std::make_shared<Executor::PriorityQueueExecutor>(clock);
    auto scheduler = std::make_shared<Scheduler::DeterministicScheduler>(executor, rng, clock);
    auto system = std::make_shared<System::System>(executor, scheduler, clock);

    std::cout << "Simulation starting" << std::endl;

    Node::Task sleep_looper_coro = Node::NodeMainLoop(scheduler, 100);
    for(int i=0;i<10000;i++) {
        system->tick();
        clock->tick();
    }

    std::cout << "Simulation finished" << std::endl;


}
