#include "src/fuzzer/fuzzer.h"
#include "src/simulation/simulation_harness.h"
#include <iostream>
#include <algorithm>
#include <iomanip>

namespace Fuzzer {

CoverageFuzzer::CoverageFuzzer(uint32_t seed) : rng_(seed) {}

void CoverageFuzzer::seed_corpus(const std::vector<uint8_t>& initial) {
    corpus_.push_back(initial);
    execute_and_update(initial);
}

std::vector<uint8_t> CoverageFuzzer::select_from_corpus() {
    if (corpus_.empty()) {
std::vector<uint8_t> input(140);
        std::uniform_int_distribution<int> dist(0, 255);
        
        for(auto& b : input) {
            b = static_cast<uint8_t>(dist(rng_));
        }
        return input;
    }
    size_t idx = rng_() % corpus_.size();
    return corpus_[idx];
}

std::vector<uint8_t> CoverageFuzzer::mutate(const std::vector<uint8_t>& input) {
    auto result = input;

    // Ensure minimum size for valid FuzzInput
    const size_t MIN_SIZE = 15;
    while (result.size() < MIN_SIZE) {
        result.push_back(rng_() % 256);
    }

    // Apply multiple mutations when coverage stalls
    int num_mutations = 1;
    if (iterations_since_new_coverage_ > 100) {
        num_mutations = 2;
    }
    if (iterations_since_new_coverage_ > 500) {
        num_mutations = 3;
    }

    for (int m = 0; m < num_mutations; ++m) {
        int strategy = rng_() % 5;

        switch (strategy) {
            case 0: { // Bit flip
                size_t pos = rng_() % (result.size() * 8);
                result[pos / 8] ^= (1 << (pos % 8));
                break;
            }
            case 1: { // Byte replace
                size_t pos = rng_() % result.size();
                result[pos] = rng_() % 256;
                break;
            }
            case 2: { // Arithmetic
                size_t pos = rng_() % result.size();
                int delta = static_cast<int>(rng_() % 35) - 17;  // -17 to +17
                result[pos] = static_cast<uint8_t>(result[pos] + delta);
                break;
            }
            case 3: { // Splice with another corpus entry
                if (corpus_.size() > 1) {
                    const auto& other = corpus_[rng_() % corpus_.size()];
                    size_t min_size = std::min(result.size(), other.size());
                    if (min_size > 0) {
                        size_t split = rng_() % min_size;
                        std::copy(other.begin(), other.begin() + split, result.begin());
                    }
                }
                break;
            }
            case 4: { // Interesting values for specific fields
                // Target the rng_seed field (first 4 bytes) with interesting values
                if (result.size() >= 4) {
                    uint32_t interesting_seeds[] = {0, 1, 0xFFFFFFFF, 0x12345678, 0xDEADBEEF};
                    uint32_t seed = interesting_seeds[rng_() % 5];
                    result[0] = (seed >> 0) & 0xFF;
                    result[1] = (seed >> 8) & 0xFF;
                    result[2] = (seed >> 16) & 0xFF;
                    result[3] = (seed >> 24) & 0xFF;
                }
                break;
            }
        }
    }

    return result;
}

bool CoverageFuzzer::execute_and_update(const std::vector<uint8_t>& input) {
    Simulation::SimulationResult result = Simulation::run_simulation(input);

    if (result.oracle_violation) {
        found_violation_ = true;
        violation_input_ = input;
        violation_message_ = result.error_message;
        return false;
    }

    // Check for new coverage
    bool found_new = false;
    for (size_t hash : result.visited_state_hashes) {
        if (global_coverage_.find(hash) == global_coverage_.end()) {
            global_coverage_.insert(hash);
            found_new = true;
        }
    }

    return found_new;
}

void CoverageFuzzer::print_violation() const {
    std::cout << violation_message_ << std::endl;
}

void CoverageFuzzer::run(size_t iterations) {
    std::cout << "[Fuzzer] Starting fuzzing run with " << iterations << " iterations" << std::endl;

    for (size_t i = 0; i < iterations; ++i) {
        // Select and mutate input
        auto base_input = select_from_corpus();
        auto mutated_input = mutate(base_input);

        // Execute and check for new coverage
        bool found_new = execute_and_update(mutated_input);

        // Stop immediately on oracle violation
        if (found_violation_) {
            std::cout << "\n[Fuzzer] Stopping at iteration " << (i + 1)
                      << " due to oracle violation\n";
            print_violation();
            return;
        }

        if (found_new) {
            corpus_.push_back(mutated_input);
            iterations_since_new_coverage_ = 0;
        } else {
            ++iterations_since_new_coverage_;
        }

        // Progress report
        if ((i + 1) % 1000 == 0 || i == 0) {
            std::cout << "[Fuzzer] Iteration " << (i + 1)
                      << ", coverage: " << global_coverage_.size() << " states"
                      << ", corpus: " << corpus_.size()
                      << ", stalled: " << iterations_since_new_coverage_
                      << std::endl;
        }
    }

    std::cout << "[Fuzzer] Fuzzing complete - no violations found" << std::endl;
}

} // namespace Fuzzer
