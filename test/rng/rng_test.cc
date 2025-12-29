#include "src/rng/rng.h"
#include "gtest/gtest.h"


TEST(SchedulerTest, AllTasksRun) {    
    for(int seed=0;seed<100;seed++) {
        RNG::UniformDistributionRange rng1(seed), rng2(seed);
        for(int j=0;j<100;j++) {
            ASSERT_EQ(rng1.draw(0,j), rng2.draw(0,j));
        }
    }

}