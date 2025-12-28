#include "src/executor/executor.h"
#include "src/clock/clock.h"
#include "gtest/gtest.h"
#include <ranges>


TEST(SchedulerTest, AllTasksRun) {
  std::vector<int> buffer;
  auto clk = std::make_shared<Clock::DeterministicClock>();
  auto pqe = std::make_shared<Executor::PriorityQueueExecutor>(clk);
  for(int i=0;i<10;i++) {
    auto lambda = [&buffer, i]() {
      buffer.push_back(i);
    };
    pqe->push_task(lambda, 10-i);
  }
  
  ASSERT_TRUE(buffer.empty());

  for(int i=1;i<=10;i++) {
    clk->tick();
    pqe->run_until_blocked();
    ASSERT_EQ(buffer.size(), i);
    ASSERT_EQ(buffer.back(), 10-i);
  }

}