#include "src/system/system.h"
#include <memory>

namespace System {

LoopTask SleeperNode::main_loop() {
    std::cout << "[Looper thread] Starting looper" << std::endl;
    for(int i=0;i<10;i++) {
        std::cout << "[Looper thread]Sleeping before co await" << std::endl;
        int delay = system_->random_range(0,100);
        co_await system_->sleep(delay);
        std::cout << "[Looper thread]Awoke after co await returned" << std::endl;
    }
    std::cout << "[Looper thread] Finished looper" << std::endl;
}

}


