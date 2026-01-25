#ifndef _FUZZER_H_
#define _FUZZER_H_

#include <vector>
#include <unordered_set>
#include <random>
#include <cstdint>
#include <string>

namespace Fuzzer {

class CoverageFuzzer {
private:
    std::unordered_set<size_t> global_coverage_;
    std::vector<std::vector<uint8_t>> corpus_;
    std::mt19937 rng_;
    size_t iterations_since_new_coverage_ = 0;

    // Violation tracking
    bool found_violation_ = false;
    std::vector<uint8_t> violation_input_;
    std::string violation_message_;

public:
    explicit CoverageFuzzer(uint32_t seed = 42);

    void seed_corpus(const std::vector<uint8_t>& initial);
    void run(size_t iterations);

    size_t coverage_count() const { return global_coverage_.size(); }
    size_t corpus_size() const { return corpus_.size(); }
    bool has_violation() const { return found_violation_; }

private:
    std::vector<uint8_t> mutate(const std::vector<uint8_t>& input);
    bool execute_and_update(const std::vector<uint8_t>& input);
    std::vector<uint8_t> select_from_corpus();
    void print_violation() const;
};

} // namespace Fuzzer

#endif // _FUZZER_H_
