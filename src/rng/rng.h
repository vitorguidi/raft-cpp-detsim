
#ifndef _RNG_H_
#define _RNG_H_

#include <random>

namespace RNG {

class RNG {
public:
    virtual ~RNG() = default;
    virtual int draw(int lo, int hi)  = 0;
};

class UniformDistributionRange : public RNG {
private:
    std::mt19937 mt_;
public:
    ~UniformDistributionRange() = default;
    UniformDistributionRange(int seed);
    int draw(int lo, int hi) override;
};


} // namespace RNG

#endif // _RNG_H_