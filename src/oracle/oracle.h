#include "src/node/node.h"
#include <vector>
#include <unordered_map>

namespace Oracle {

class Oracle {
public:
    virtual void enforce_invariants() = 0;
};

class RaftOracle : public Oracle {
private:
    std::vector<std::shared_ptr<Node::RaftNode>> nodes_;
    std::unordered_map<int, int> leader_per_term_;  // term -> node_id
    void enforce_election_safety();
public:
    void enforce_invariants() override;
    RaftOracle(std::vector<std::shared_ptr<Node::Node>> nodes) {
        for(auto node : nodes) {
            auto casted_ptr = std::dynamic_pointer_cast<Node::RaftNode>(node);
            if (!casted_ptr) {
                throw std::runtime_error("Unable to cast Node into RaftNode");
            }
            nodes_.push_back(casted_ptr);
        }

    }
};

}