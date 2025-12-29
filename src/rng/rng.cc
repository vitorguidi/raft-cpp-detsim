#include "src/rng/rng.h"

namespace RNG {

UniformDistributionRange::UniformDistributionRange(int seed) : mt_(seed) {}

int UniformDistributionRange::draw(int lo, int hi) {
    std::uniform_int_distribution dist(lo, hi);
    return dist(mt_);
}
} // namespace RNG