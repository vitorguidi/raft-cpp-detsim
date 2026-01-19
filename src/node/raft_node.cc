#include "src/node/node.h"
#include "src/system/system.h"
#include <memory>
#include <iostream>

namespace Node {

Task RaftNode::main_loop() {
    std::cout << "[Node " << std::to_string(id_) << "] Starting main raft loop" << std::endl;
    for(int i=0;i<nr_nodes_;i++) {
        std::cout 
            << "[Node " << std::to_string(id_)
            << "] Sending AppendEntriesRequest to node "
            << std::to_string(i)
            << std::endl;
        auto append_entries_req = IO::AppendEntriesRequest{
            .term = term_,
            .leader_id = id_,
        };
        auto resp = co_await system_->rpc(id_, i, IO::MessageName::APPEND_ENTRIES_REQUEST, append_entries_req);
        auto content = std::get<IO::AppendEntriesResponse>(resp.content);
        std::cout 
            << "[Node " << std::to_string(id_) 
            << "] Received AppendEntriesResponse from node " 
            << resp.from << ": term = " << std::to_string(content.term)
            << std::endl;
        std::cout << "[Node " << std::to_string(id_) << "] Sleeping a bit " << std::endl;
        co_await system_->sleep(50);
        std::cout << "[Node " << std::to_string(id_) << "] Woke up " << std::endl;
        auto vote_request = IO::RequestVoteRequest{
            .term = term_,
            .candidate_id = id_,
        };
        std::cout 
            << "[Node " << std::to_string(id_)
            << "] Sending RequestVoteRequest to node "
            << std::to_string(i)
            << std::endl;
        auto vote_resp = co_await system_->rpc(id_, i, IO::MessageName::REQUEST_VOTE_REQUEST, vote_request);
        auto vote_content = std::get<IO::RequestVoteResponse>(vote_resp.content);
        std::cout 
            << "[Node " << std::to_string(id_) 
            << "] Received AppendEntriesResponse from node " 
            << resp.from << ": term = " << std::to_string(content.term)
            << std::endl;
    }
    co_return;
}


Task RaftNode::handle_append_entries(IO::Envelope msg) {
    if (msg.to != id_) {
        throw std::runtime_error("Message got to the wrong node: was meant for " + std::to_string(msg.to) + ", but arrived to " + std::to_string(id_));
    }
    if (msg.name != IO::MessageName::APPEND_ENTRIES_REQUEST) {
        throw std::runtime_error("Wrong request got to the append entries handler");
    }
    auto response = IO::Envelope {
        .message_id = msg.message_id,
        .name = IO::MessageName::APPEND_ENTRIES_RESPONSE,
        .from = id_,
        .to = msg.from,
        .content = IO::AppendEntriesResponse{
            .term = term_,
        }
    };
    std::cout << "[Node " << std::to_string(id_) << "] Sent AppendEntriesResponse to node" << std::to_string(msg.from) << std::endl;
    system_->send_message(response);
    co_return;
}

Task RaftNode::handle_request_vote(IO::Envelope msg) {
    if (msg.to != id_) {
        throw std::runtime_error("Message got to the wrong node: was meant for " + std::to_string(msg.to) + ", but arrived to " + std::to_string(id_));
    }
    if (msg.name != IO::MessageName::REQUEST_VOTE_REQUEST) {
        throw std::runtime_error("Wrong request got to the request vote handler");
    }
    auto response = IO::Envelope {
        .message_id = msg.message_id,
        .name = IO::MessageName::REQUEST_VOTE_RESPONSE,
        .from = id_,
        .to = msg.from,
        .content = IO::RequestVoteResponse{
            .term = term_,
            .vote_granted = false,
        }
    };
    std::cout << "[Node " << std::to_string(id_) << "] Sent RequestVoteResponse to node" << std::to_string(msg.from) << std::endl;
    system_->send_message(response);
    co_return;
}

void RaftNode::dispatch() {
    std::cout << "[Node " << std::to_string(id_) << "] Dispatching messages" << std::endl;
    for(auto msg : inbox) {
        switch(msg.name) {
            case IO::MessageName::REQUEST_VOTE_REQUEST: {
                auto handler_coro = handle_request_vote(msg);
                auto resumer_lambda = [handler_coro]() {handler_coro.h_.resume();};
                std::cout << "[Node " << std::to_string(id_) << "] Dispatching REQUEST_VOTE_REQUEST with id = " << std::to_string(msg.message_id) << std::endl;
                system_->request_work(resumer_lambda);
                break;
            }
            case IO::MessageName::APPEND_ENTRIES_REQUEST: {
                auto handler_coro = handle_append_entries(msg);
                auto resumer_lambda = [handler_coro]() {handler_coro.h_.resume();};
                std::cout << "[Node " << std::to_string(id_) << "] Dispatching APPEND_ENTRIES_REQUEST with id = " << std::to_string(msg.message_id) << std::endl;
                system_->request_work(resumer_lambda);
                break;
            }
            case IO::MessageName::REQUEST_VOTE_RESPONSE:
                std::cout << "[Node " << std::to_string(id_) << "] Resuming REQUEST_VOTE_RESPONSE, msg id = " << std::to_string(msg.message_id) << std::endl;
                system_->register_rpc_completion(msg.message_id, msg);
                break;
            case IO::MessageName::APPEND_ENTRIES_RESPONSE:
                std::cout << "[Node " << std::to_string(id_) << "] Resuming APPEND_ENTRIES_RESPONSE, msg id = " << std::to_string(msg.message_id) << std::endl;
                system_->register_rpc_completion(msg.message_id, msg);
                break;
            default:
                throw std::runtime_error("Unsupported message type for raft node.");
        }
    }
    inbox.clear();
}

}