#include "src/rng/rng.h"
#include "gtest/gtest.h"


TEST(SchedulerTest, AllTasksRun) {    
    for(int seed=0;seed<100;seed++) {
        RNG::UniformDistributionRange rng1(seed, 0, 100), rng2(seed, 0, 100);
        for(int j=0;j<100;j++) {
            ASSERT_EQ(rng1.draw(), rng2.draw());
        }
    }

}