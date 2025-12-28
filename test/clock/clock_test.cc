#include "src/clock/clock.h"
#include "gtest/gtest.h"


TEST(SchedulerTest, AllTasksRun) {
  auto clk = std::make_shared<Clock::DeterministicClock>();
  for(int i=0;i<300;i++) {
    ASSERT_EQ(clk->now(), i);
    clk->tick();
  }
}