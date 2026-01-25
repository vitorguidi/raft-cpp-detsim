#ifndef _SIMULATION_HARNESS_H_
#define _SIMULATION_HARNESS_H_

#include <unordered_set>
#include <string>
#include "src/simulation/fuzz_input.h"

namespace Simulation {

struct SimulationResult {
    std::unordered_set<size_t> visited_state_hashes;
    bool oracle_violation = false;
    std::string error_message;
};

SimulationResult run_simulation(const std::vector<uint8_t>& input);

} // namespace Simulation

#endif // _SIMULATION_HARNESS_H_
