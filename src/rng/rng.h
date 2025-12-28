
#ifndef _RNG_H_
#define _RNG_H_

#include <random>

namespace RNG {

class RNG {
public:
    virtual ~RNG() = default;
    virtual int draw()  = 0;
};

class UniformDistributionRange : public RNG {
private:
    std::mt19937 mt_;
    std::uniform_int_distribution<int> dist_;
public:
    ~UniformDistributionRange() = default;
    UniformDistributionRange(int seed, int lo, int hi)
        : mt_(seed), dist_(lo, hi) {}
    int draw() override {
        return dist_(mt_);
    }

};


} // namespace RNG

#endif // _RNG_H_