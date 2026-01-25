#include "src/simulation/simulation_harness.h"
#include "src/rng/rng.h"
#include "src/clock/clock.h"
#include "src/executor/executor.h"
#include "src/scheduler/scheduler.h"
#include "src/io/network.h"
#include "src/system/system.h"
#include "src/node/node.h"
#include "src/routing/router.h"
#include "src/oracle/oracle.h"
#include "src/node/state.h"
#include <stdexcept>
#include <iostream>
#include <sstream>

namespace Simulation {

// RAII helper to suppress cout during simulation
class SuppressOutput {
    std::streambuf* original_cout_;
    std::ostringstream null_stream_;
public:
    SuppressOutput() : original_cout_(std::cout.rdbuf()) {
        std::cout.rdbuf(null_stream_.rdbuf());
    }
    ~SuppressOutput() {
        std::cout.rdbuf(original_cout_);
    }
};

std::string generate_violation_message(const std::string& exception_message, FuzzInput violation_input_) {
    std::string msg = "";
    msg+= "\n";
    msg+= "║                    ORACLE VIOLATION FOUND!                   ║\n";
    msg+= "╔══════════════════════════════════════════════════════════════╗\n";
    msg+= "╚══════════════════════════════════════════════════════════════╝\n";
    msg+= "\n";
    msg+= "Error: " + exception_message + "\n";
    msg+= "\n";
    msg+= "=== Reproduction Parameters ===\n";
    msg+= "rng_seed:             " + std::to_string(violation_input_.rng_seed) + "\n";
    msg+= "nr_nodes:             " + std::to_string(violation_input_.nr_nodes) + "\n";
    msg+= "election_timeout_min: " + std::to_string(violation_input_.election_timeout_min) + "\n";
    msg+= "election_timeout_max: " + std::to_string(violation_input_.election_timeout_max) + "\n";
    msg+= "heartbeat_interval:   " + std::to_string(violation_input_.heartbeat_interval) + "\n";
    msg+= "max_network_delay:    " + std::to_string(violation_input_.max_network_delay) + "\n";
    msg+= "max_steps:            " + std::to_string(violation_input_.max_steps) + "\n";
    msg+= "\n";
    msg+= "=== Raw Input Bytes (hex) ===\n";
    return msg;
}

SimulationResult run_simulation(const std::vector<uint8_t>& fuzz_input) {
    FuzzInput input = FuzzInput::from_bytes(fuzz_input.data(), fuzz_input.size());

    SimulationResult result;

    try {
        // Suppress verbose debug output from simulation components
        SuppressOutput suppress;
        // Initialize core components with fuzz input parameters
        auto rng = std::make_shared<RNG::UniformDistributionRange>(input.rng_seed);
        auto clock = std::make_shared<Clock::DeterministicClock>();
        auto executor = std::make_shared<Executor::PriorityQueueExecutor>(clock);

        const int MAX_TASK_SCHEDULE_JITTER = 100;
        auto scheduler = std::make_shared<Scheduler::DeterministicScheduler>(
            executor, rng, clock, MAX_TASK_SCHEDULE_JITTER);
        auto network = std::make_shared<IO::Network>(clock, rng, input.max_network_delay);
        auto system = std::make_shared<System::System>(scheduler, clock, rng, network);

        // Create nodes
        std::vector<std::shared_ptr<Node::Node>> nodes;
        std::vector<std::shared_ptr<Node::RaftNode>> raft_nodes;

        for (int i = 0; i < input.nr_nodes; ++i) {
            auto node = std::make_shared<Node::RaftNode>(
                i, system, input.nr_nodes,
                input.election_timeout_min,
                input.election_timeout_max,
                input.heartbeat_interval);
            nodes.push_back(node);
            raft_nodes.push_back(node);

            auto main_loop = node->main_loop();
            system->request_work([main_loop]() { main_loop.h_.resume(); });
        }

        // Set up routing and oracle
        auto router = std::make_shared<Routing::Router>(nodes, network, system);
        auto oracle = std::make_shared<Oracle::RaftOracle>(nodes);

        // Run simulation loop
        int steps = 0;
        while ((executor->has_work() || network->has_messages()) && steps < input.max_steps) {
            clock->tick();
            router->route();
            executor->run_until_blocked();

            // Capture state after each step
            auto cluster_state = State::ClusterState::capture(raft_nodes);
            result.visited_state_hashes.insert(cluster_state.hash());

            // Check oracle invariants
            oracle->enforce_invariants();

            ++steps;
        }

    } catch (const std::runtime_error& e) {
        result.oracle_violation = true;
        result.error_message = generate_violation_message(e.what(),input);
    } catch (const std::exception& e) {
        result.oracle_violation = true;
        result.error_message = std::string("Unexpected error: ") + e.what();
    }

    return result;
}

} // namespace Simulation
