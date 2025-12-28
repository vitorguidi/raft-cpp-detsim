#ifndef _CLOCK_H_
#define _CLOCK_H_

namespace Clock {
class Clock {
public:
    virtual long long int now() = 0;
    virtual void tick() = 0;
    virtual ~Clock() = default;
};
class DeterministicClock : public Clock {
public:
    void tick() override {time_++;}
    long long int now() override {return time_;}
    ~DeterministicClock() = default;
    DeterministicClock() : time_(0) {}
private:
    long long int time_ = 0;
};
} // namespace Clock
#endif // _CLOCK_H_