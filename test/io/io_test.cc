#include "src/executor/executor.h"
#include "src/clock/clock.h"
#include "src/rng/rng.h"
#include "src/io/network.h"

#include "gtest/gtest.h"
#include <ranges>
#include <stdexcept>

TEST(IoTest, NetworkPushesAndSendsOnExpectedOrder) {
    auto clk = std::make_shared<Clock::DeterministicClock>();

    class FixedRNG : public RNG::RNG {
    private:
        std::vector<int> draws_;
        size_t pos_;
    public:
        FixedRNG(std::vector<int>& draws) : draws_(std::move(draws)), pos_(0) {}
        int draw(int lo, int hi) {
            if (pos_ >= draws_.size()) {
                throw std::runtime_error("Drew FixedRNG beyond number of provided mocks.");
            }
            return draws_[pos_++];
        }
    };

    int max_delay = 10;
    std::vector<int> fixed_draws = {max_delay/2,max_delay,max_delay/2};
    auto rng = std::make_shared<FixedRNG>(fixed_draws);

    auto network = std::make_shared<IO::Network>(clk, rng, max_delay);

    auto msg1 = IO::Envelope{
        0,
        IO::MessageName::PING_REQUEST,
        0,
        1,
        IO::PingRequest{}
    };
    auto msg2 = IO::Envelope{
        1,
        IO::MessageName::PING_REQUEST,
        0,
        2,
        IO::PingResponse{}
    };
    auto msg3 = IO::Envelope{
        2,
        IO::MessageName::PING_RESPONSE,
        2,
        0,
        IO::PingResponse{}
    };
    network->push_entry(msg1);
    network->push_entry(msg2);
    network->push_entry(msg3);
    for(int i=0;i<=max_delay;i++) {
        clk->tick();
    }

    auto fetched_msgs = network->fetch_ready();
    ASSERT_EQ(fetched_msgs[0], msg1); //must be first because it is on the lower timestamp and lower msg id
    ASSERT_EQ(fetched_msgs[1], msg3); // must be second because is on the lower timestamp, but higher msg id
    ASSERT_EQ(fetched_msgs[2], msg2); // must be last because is on the higher timestamp
}