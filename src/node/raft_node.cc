#include "src/node/node.h"
#include "src/system/system.h"
#include <memory>
#include <iostream>

namespace Node {

Task RaftNode::main_loop() {
    std::cout << "[Node " << id_ << "] Starting main raft loop as FOLLOWER" << std::endl;
    last_heartbeat_time_ = system_->get_time();

    while (true) {
        if (state_ == LEADER) {
            // Send heartbeats to all other nodes
            for (int i = 0; i < nr_nodes_; i++) {
                if (i == id_) continue;
                std::cout << "[Node " << id_ << "] Sending heartbeat to node " << i << std::endl;
                auto append_entries_req = IO::AppendEntriesRequest{
                    .term = term_,
                    .leader_id = id_,
                };
                auto resp = co_await system_->rpc(id_, i, IO::MessageName::APPEND_ENTRIES_REQUEST, append_entries_req);
                auto content = std::get<IO::AppendEntriesResponse>(resp.content);

                // If we get a higher term, step down
                if (content.term > term_) {
                    std::cout << "[Node " << id_ << "] Received higher term " << content.term
                              << " from node " << resp.from << ", stepping down" << std::endl;
                    term_ = content.term;
                    state_ = FOLLOWER;
                    voted_for_ = std::nullopt;
                    last_heartbeat_time_ = system_->get_time();
                    break;
                }
            }
            if (state_ == LEADER) {
                co_await system_->sleep(heartbeat_interval_);
            }
        } else {
            // FOLLOWER or CANDIDATE
            int election_timeout = system_->random_range(election_timeout_min_, election_timeout_max_);
            long long current_time = system_->get_time();
            long long elapsed = current_time - last_heartbeat_time_;

            if (elapsed >= election_timeout) {
                // Start election
                state_ = CANDIDATE;
                term_++;
                voted_for_ = id_;
                votes_received_ = 1;  // Vote for self
                last_heartbeat_time_ = system_->get_time();

                std::cout << "[Node " << id_ << "] Election timeout, becoming CANDIDATE for term " << term_ << std::endl;

                // Request votes from all other nodes
                for (int i = 0; i < nr_nodes_; i++) {
                    if (i == id_) continue;

                    // Check if we're still a candidate (could have stepped down while waiting)
                    if (state_ != CANDIDATE) {
                        std::cout << "[Node " << id_ << "] No longer a candidate, aborting election" << std::endl;
                        break;
                    }

                    std::cout << "[Node " << id_ << "] Requesting vote from node " << i << std::endl;
                    auto vote_request = IO::RequestVoteRequest{
                        .term = term_,
                        .candidate_id = id_,
                    };
                    auto vote_resp = co_await system_->rpc(id_, i, IO::MessageName::REQUEST_VOTE_REQUEST, vote_request);
                    auto vote_content = std::get<IO::RequestVoteResponse>(vote_resp.content);

                    std::cout << "[Node " << id_ << "] Received vote response from node " << vote_resp.from
                              << ": term=" << vote_content.term << ", granted=" << vote_content.vote_granted << std::endl;

                    // Check if we're still a candidate after receiving response
                    if (state_ != CANDIDATE) {
                        std::cout << "[Node " << id_ << "] No longer a candidate after receiving response, aborting election" << std::endl;
                        break;
                    }

                    // If we get a higher term, step down
                    if (vote_content.term > term_) {
                        std::cout << "[Node " << id_ << "] Received higher term " << vote_content.term
                                  << ", stepping down to FOLLOWER" << std::endl;
                        term_ = vote_content.term;
                        state_ = FOLLOWER;
                        voted_for_ = std::nullopt;
                        last_heartbeat_time_ = system_->get_time();
                        break;
                    }

                    // Only count votes for our current term
                    if (vote_content.term == term_ && vote_content.vote_granted) {
                        votes_received_++;
                        std::cout << "[Node " << id_ << "] Got vote, now have " << votes_received_ << " votes" << std::endl;

                        // Check if we have majority
                        if (votes_received_ > nr_nodes_ / 2) {
                            std::cout << "[Node " << id_ << "] Won election with " << votes_received_
                                      << " votes, becoming LEADER for term " << term_ << std::endl;
                            state_ = LEADER;
                            break;
                        }
                    } else if (vote_content.term < term_) {
                        std::cout << "[Node " << id_ << "] Ignoring stale vote response from term "
                                  << vote_content.term << " (current term: " << term_ << ")" << std::endl;
                    }
                }

                // If we didn't win and are still candidate, we'll timeout and try again
                if (state_ == CANDIDATE) {
                    std::cout << "[Node " << id_ << "] Election failed, will retry" << std::endl;
                }
            } else {
                // Sleep a bit and check again
                co_await system_->sleep(10);
            }
        }
    }
}


Task RaftNode::handle_append_entries(IO::Envelope msg) {
    if (msg.to != id_) {
        throw std::runtime_error("Message got to the wrong node: was meant for " + std::to_string(msg.to) + ", but arrived to " + std::to_string(id_));
    }
    if (msg.name != IO::MessageName::APPEND_ENTRIES_REQUEST) {
        throw std::runtime_error("Wrong request got to the append entries handler");
    }

    auto request = std::get<IO::AppendEntriesRequest>(msg.content);

    // If the leader's term is higher, update our term and reset vote
    if (request.term > term_) {
        term_ = request.term;
        state_ = FOLLOWER;
        voted_for_ = std::nullopt;
        last_heartbeat_time_ = system_->get_time();
        std::cout << "[Node " << id_ << "] Received AppendEntries from leader " << request.leader_id
                  << " for higher term " << request.term << ", stepping down" << std::endl;
    } else if (request.term == term_) {
        // Same term - recognize the leader but DON'T reset voted_for
        state_ = FOLLOWER;
        last_heartbeat_time_ = system_->get_time();
        std::cout << "[Node " << id_ << "] Received AppendEntries from leader " << request.leader_id
                  << " for term " << request.term << ", resetting heartbeat" << std::endl;
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
    std::cout << "[Node " << id_ << "] Sent AppendEntriesResponse to node " << msg.from << std::endl;
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

    auto request = std::get<IO::RequestVoteRequest>(msg.content);
    bool vote_granted = false;

    // If the candidate's term is higher, update our term and reset vote
    if (request.term > term_) {
        std::cout << "[Node " << id_ << "] RequestVote from " << request.candidate_id
                  << " has higher term " << request.term << ", updating" << std::endl;
        term_ = request.term;
        state_ = FOLLOWER;
        voted_for_ = std::nullopt;
    }

    // Grant vote if: term is current and we haven't voted or already voted for this candidate
    if (request.term >= term_ && (!voted_for_.has_value() || voted_for_.value() == request.candidate_id)) {
        voted_for_ = request.candidate_id;
        vote_granted = true;
        last_heartbeat_time_ = system_->get_time();
        std::cout << "[Node " << id_ << "] Granting vote to candidate " << request.candidate_id
                  << " for term " << request.term << std::endl;
    } else {
        std::cout << "[Node " << id_ << "] Denying vote to candidate " << request.candidate_id
                  << " for term " << request.term << " (our term=" << term_
                  << ", voted_for=" << (voted_for_.has_value() ? std::to_string(voted_for_.value()) : "none") << ")" << std::endl;
    }

    auto response = IO::Envelope {
        .message_id = msg.message_id,
        .name = IO::MessageName::REQUEST_VOTE_RESPONSE,
        .from = id_,
        .to = msg.from,
        .content = IO::RequestVoteResponse{
            .term = term_,
            .vote_granted = vote_granted,
        }
    };
    std::cout << "[Node " << id_ << "] Sent RequestVoteResponse to node " << msg.from << std::endl;
    system_->send_message(response);
    co_return;
}

void RaftNode::dispatch() {
    std::cout << "[Node " << id_ << "] Dispatching messages" << std::endl;
    for(auto msg : inbox) {
        switch(msg.name) {
            case IO::MessageName::REQUEST_VOTE_REQUEST: {
                auto handler_coro = handle_request_vote(msg);
                auto resumer_lambda = [handler_coro]() {handler_coro.h_.resume();};
                std::cout << "[Node " << id_ << "] Dispatching REQUEST_VOTE_REQUEST with id = " << msg.message_id << std::endl;
                system_->request_work(resumer_lambda);
                break;
            }
            case IO::MessageName::APPEND_ENTRIES_REQUEST: {
                auto handler_coro = handle_append_entries(msg);
                auto resumer_lambda = [handler_coro]() {handler_coro.h_.resume();};
                std::cout << "[Node " << id_ << "] Dispatching APPEND_ENTRIES_REQUEST with id = " << msg.message_id << std::endl;
                system_->request_work(resumer_lambda);
                break;
            }
            case IO::MessageName::REQUEST_VOTE_RESPONSE:
                std::cout << "[Node " << id_ << "] Resuming REQUEST_VOTE_RESPONSE, msg id = " << msg.message_id << std::endl;
                system_->register_rpc_completion(msg.message_id, msg);
                break;
            case IO::MessageName::APPEND_ENTRIES_RESPONSE:
                std::cout << "[Node " << id_ << "] Resuming APPEND_ENTRIES_RESPONSE, msg id = " << msg.message_id << std::endl;
                system_->register_rpc_completion(msg.message_id, msg);
                break;
            default:
                throw std::runtime_error("Unsupported message type for raft node.");
        }
    }
    inbox.clear();
}

}
