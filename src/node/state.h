#ifndef _STATE_H_
#define _STATE_H_

#include <vector>
#include <optional>
#include <functional>
#include "src/node/node.h"

namespace State {

struct NodeState {
    int id;
    Node::RaftState state;
    int term;
    std::optional<int> voted_for;
    int votes_received;

    bool operator==(const NodeState& other) const;
    size_t hash() const;
};

struct ClusterState {
    std::vector<NodeState> nodes;

    size_t hash() const;
    bool operator==(const ClusterState& other) const;

    static ClusterState capture(const std::vector<std::shared_ptr<Node::RaftNode>>& nodes);
};

} // namespace State

#endif // _STATE_H_
