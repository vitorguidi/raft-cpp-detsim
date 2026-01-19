#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <coroutine>
#include <vector>
#include <queue>
#include <algorithm>
#include "src/io/messages.h"
#include "src/clock/clock.h"
#include "src/rng/rng.h"
#include <memory>

namespace IO {

struct NetworkItem {
    IO::Envelope envelope;
    long long int arrival_time;
    bool operator>(const NetworkItem& other) const {
        if (arrival_time != other.arrival_time) {
            return arrival_time > other.arrival_time;
        }
        return envelope.message_id > other.envelope.message_id;
    }
};

class Network {
private:
    std::shared_ptr<Clock::Clock> clock_;
    std::shared_ptr<RNG::RNG> rng_;
    std::priority_queue<NetworkItem, std::vector<NetworkItem>, std::greater<NetworkItem>> wire_;
    int max_delay_;
public:
    Network(std::shared_ptr<Clock::Clock> clock, std::shared_ptr<RNG::RNG> rng, int max_delay)
        : clock_(clock),
          rng_(rng),
          max_delay_(max_delay) {}
    void push_entry(IO::Envelope msg) {
        int delay = rng_->draw(0, max_delay_);
        NetworkItem entry = NetworkItem{msg, clock_->now() + delay};
        wire_.push(entry);
    }
    bool has_messages() {return !wire_.empty();}
    std::vector<IO::Envelope> fetch_ready() {
        std::vector<IO::Envelope> results;
        while(!wire_.empty() && wire_.top().arrival_time <= clock_->now() ) {
            results.push_back(wire_.top().envelope);
            wire_.pop();
        }
        return results;
    }
    
};

} // namespace IO

#endif // _NETWORK_H_