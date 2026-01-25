#include "src/oracle/oracle.h"
#include "src/node/node.h"
#include <iostream>
#include <stdexcept>

namespace Oracle {

void RaftOracle::enforce_election_safety() {
    // Election safety invariant: At most one leader can be elected per term
    for (const auto& node : nodes_) {
        if (node->get_state() == Node::LEADER) {
            int term = node->get_term();
            int node_id = node->id_;

            auto it = leader_per_term_.find(term);
            if (it != leader_per_term_.end()) {
                if (it->second != node_id) {
                    throw std::runtime_error(
                        "ELECTION SAFETY VIOLATION: Two leaders in term " + std::to_string(term) +
                        "! Node " + std::to_string(it->second) + " and Node " + std::to_string(node_id) +
                        " are both leaders.");
                }
            } else {
                leader_per_term_[term] = node_id;
                std::cout << "[Oracle] Recorded leader " << node_id << " for term " << term << std::endl;
            }
        }
    }
}

void RaftOracle::enforce_invariants() {
    enforce_election_safety();
}

};
