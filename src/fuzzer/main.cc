#include "src/fuzzer/fuzzer.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    size_t iterations = 10000;
    uint32_t fuzzer_seed = 42;

    if (argc > 1) {
        iterations = std::stoull(argv[1]);
    }
    if (argc > 2) {
        fuzzer_seed = std::stoul(argv[2]);
    }

    std::cout << "Raft Fuzzer - Coverage-guided state space exploration" << std::endl;
    std::cout << "Iterations: " << iterations << ", Fuzzer seed: " << fuzzer_seed << std::endl;
    std::cout << std::endl;

    Fuzzer::CoverageFuzzer fuzzer(fuzzer_seed);
    // Run fuzzer
    fuzzer.run(iterations);

    std::cout << std::endl;
    std::cout << "=== Final Results ===" << std::endl;
    std::cout << "Coverage: " << fuzzer.coverage_count() << " unique states" << std::endl;
    std::cout << "Corpus: " << fuzzer.corpus_size() << " interesting inputs" << std::endl;

    return 0;
}
