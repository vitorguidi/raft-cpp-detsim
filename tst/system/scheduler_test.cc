#include "src/system/scheduler.h"
#include "gtest/gtest.h"
#include <thread>

// Demonstrate some basic assertions.
TEST(SchedulerTest, EmptyThreadPoolNotAllowed) {
  ASSERT_THROW(
    Executor ex(0),
    std::invalid_argument
  );
}

TEST(SchedulerTest, AllTasksRun) {
  const int NUM_THREADS=5;
  const int NUM_TASKS=30;
  std::atomic<int> value(0);
  {
    Executor tp(NUM_THREADS);
    for(int i=0;i<NUM_TASKS;i++) {
      tp.push_task([&value](){
        value++;
      });
    }
  }
  ASSERT_EQ(NUM_TASKS, value);
}