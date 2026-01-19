#include "src/node/node.h"
#include "src/system/system.h"
#include <memory>
#include <iostream>

namespace Node {

Task PingerNode::main_loop() {
    std::cout << "[Node " << std::to_string(id_) << "] Starting main loop" << std::endl;
    for(int i=0;i<10;i++) {
        int destination_node = system_->random_range(0, nr_nodes_-1);
        std::cout << "[Node " << std::to_string(id_) << "] Pinging node " << std::to_string(destination_node) << std::endl;
        co_await system_->rpc(id_, destination_node, IO::MessageName::PING_REQUEST, IO::PingRequest{});
        std::cout << "[Node " << std::to_string(id_) << "] Got ping from node " << std::to_string(destination_node) << std::endl;
    }
}


Task PingerNode::handle_ping(IO::Envelope msg) {
    if (msg.to != id_) {
        throw std::runtime_error("Message got to the wrong node: was meant for " + std::to_string(msg.to) + ", but arrived to " + std::to_string(id_));
    }
    if (msg.name != IO::MessageName::PING_REQUEST) {
        throw std::runtime_error("Wrong request got to the ping handler");
    }
    std::cout << "Dispatching messages in node " << std::to_string(id_) << std::endl;
    auto response = IO::Envelope {
        .message_id = msg.message_id,
        .name = IO::MessageName::PING_RESPONSE,
        .from = id_,
        .to = msg.from,
        .content = IO::PingResponse{}
    };
    system_->send_message(response);
    std::cout << "[Node " << std::to_string(id_) <<  "] Sent ping response to node " << std::to_string(response.to) << std::endl;
    co_return;
}

void PingerNode::dispatch() {
    std::cout << "[Node " << std::to_string(id_) << "] Dispatching messages" << std::endl;
    for(auto msg : inbox) {
        switch(msg.name) {
            case IO::MessageName::PING_REQUEST: {
                auto handler_coro = handle_ping(msg);
                auto resumer_lambda = [handler_coro]() {handler_coro.h_.resume();};
                std::cout << "[Node " << std::to_string(id_) << "] Dispatching Ping request with id = " << std::to_string(msg.message_id) << std::endl;
                system_->request_work(resumer_lambda);
                break;
            }
            case IO::MessageName::PING_RESPONSE:
                std::cout << "[Node " << std::to_string(id_) << "] Resuming Ping response, msg id = " << std::to_string(msg.message_id) << std::endl;
                system_->register_rpc_completion(msg.message_id, msg);
                break;
            default:
                throw std::runtime_error("Unsupported message type for pinger node.");
        }
    }
    inbox.clear();

}

}