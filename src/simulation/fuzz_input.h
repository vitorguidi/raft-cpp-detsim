#ifndef _FUZZ_INPUT_H_
#define _FUZZ_INPUT_H_

#include <cstdint>
#include <vector>

namespace Simulation {

struct FuzzInput {
    uint32_t rng_seed = 0;
    int nr_nodes = 5;
    int election_timeout_min = 150;
    int election_timeout_max = 300;
    int heartbeat_interval = 50;
    int max_network_delay = 100;
    int max_steps = 10000;

    static FuzzInput from_bytes(const uint8_t* data, size_t size);
    std::vector<uint8_t> to_bytes() const;

    // Validate and clamp parameters to reasonable ranges
    void normalize();
};

} // namespace Simulation

#endif // _FUZZ_INPUT_H_
