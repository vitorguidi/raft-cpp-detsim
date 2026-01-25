#include "src/simulation/fuzz_input.h"
#include <cstring>
#include <algorithm>

namespace Simulation {

FuzzInput FuzzInput::from_bytes(const uint8_t* data, size_t size) {
    FuzzInput input;

    // Read fields from byte array with bounds checking
    size_t offset = 0;

    auto read_uint32 = [&]() -> uint32_t {
        if (offset + sizeof(uint32_t) > size) return 0;
        uint32_t val;
        std::memcpy(&val, data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        return val;
    };

    auto read_uint16 = [&]() -> uint16_t {
        if (offset + sizeof(uint16_t) > size) return 0;
        uint16_t val;
        std::memcpy(&val, data + offset, sizeof(uint16_t));
        offset += sizeof(uint16_t);
        return val;
    };

    auto read_uint8 = [&]() -> uint8_t {
        if (offset + sizeof(uint8_t) > size) return 0;
        uint8_t val = data[offset];
        offset += sizeof(uint8_t);
        return val;
    };

    input.rng_seed = read_uint32();
    input.nr_nodes = read_uint8();
    input.election_timeout_min = read_uint16();
    input.election_timeout_max = read_uint16();
    input.heartbeat_interval = read_uint16();
    input.max_network_delay = read_uint16();
    input.max_steps = read_uint16();

    input.normalize();
    return input;
}

std::vector<uint8_t> FuzzInput::to_bytes() const {
    std::vector<uint8_t> bytes;
    bytes.reserve(15); // 4 + 1 + 2*5 = 15 bytes

    auto write_uint32 = [&](uint32_t val) {
        bytes.push_back((val >> 0) & 0xFF);
        bytes.push_back((val >> 8) & 0xFF);
        bytes.push_back((val >> 16) & 0xFF);
        bytes.push_back((val >> 24) & 0xFF);
    };

    auto write_uint16 = [&](uint16_t val) {
        bytes.push_back((val >> 0) & 0xFF);
        bytes.push_back((val >> 8) & 0xFF);
    };

    auto write_uint8 = [&](uint8_t val) {
        bytes.push_back(val);
    };

    write_uint32(rng_seed);
    write_uint8(static_cast<uint8_t>(nr_nodes));
    write_uint16(static_cast<uint16_t>(election_timeout_min));
    write_uint16(static_cast<uint16_t>(election_timeout_max));
    write_uint16(static_cast<uint16_t>(heartbeat_interval));
    write_uint16(static_cast<uint16_t>(max_network_delay));
    write_uint16(static_cast<uint16_t>(max_steps));

    return bytes;
}

void FuzzInput::normalize() {
    // Clamp node count to valid range (3-7 for meaningful Raft behavior)
    nr_nodes = std::clamp(nr_nodes, 3, 7);

    // Ensure timeouts are positive and reasonable
    election_timeout_min = std::clamp(election_timeout_min, 50, 1000);
    election_timeout_max = std::clamp(election_timeout_max, election_timeout_min + 1, 2000);

    // Heartbeat must be less than election timeout for liveness
    heartbeat_interval = std::clamp(heartbeat_interval, 10, election_timeout_min / 2);

    // Network delay must be reasonable
    max_network_delay = std::clamp(max_network_delay, 1, 500);

    // Simulation steps bounded
    max_steps = std::clamp(max_steps, 100, 50000);
}

} // namespace Fuzzer
