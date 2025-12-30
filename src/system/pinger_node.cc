#include "src/system/system.h"
#include <memory>

namespace System {

LoopTask PingerNode::main_loop() {
    std::cout << "[Node " << id_ << "] Starting pinging around " << std::endl;
    for(int i=0;i<30;i++) {
        int tgt = i%system_->nodes_.size();
        std::cout << "[Node " << id_ << "] Pinging node " << tgt << std::endl;
        PingResponse pong = co_await System::RPC(id_, tgt, PingRequest{id_}, system_);
        std::cout << "[Node " << id_ << "] Got pong from node " << pong.from << std::endl;
    }
        std::cout << "[Node " << id_ << "] Finished Pinging around" << std::endl;
}

RPCTask PingerNode::handle_rpc(PingRequest request) {
    co_return PingResponse(id_);
}

}