#include "src/node/state.h"

namespace State {

bool NodeState::operator==(const NodeState& other) const {
    return id == other.id &&
           state == other.state &&
           term == other.term &&
           voted_for == other.voted_for &&
           votes_received == other.votes_received;
}

size_t NodeState::hash() const {
    size_t h = std::hash<int>{}(id);
    h ^= std::hash<int>{}(static_cast<int>(state)) << 1;
    h ^= std::hash<int>{}(term) << 2;
    h ^= std::hash<int>{}(voted_for.value_or(-1)) << 3;
    h ^= std::hash<int>{}(votes_received) << 4;
    return h;
}

bool ClusterState::operator==(const ClusterState& other) const {
    if (nodes.size() != other.nodes.size()) return false;
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (!(nodes[i] == other.nodes[i])) return false;
    }
    return true;
}

size_t ClusterState::hash() const {
    size_t h = 0;
    for (const auto& node : nodes) {
        // Combine hashes with rotation to preserve ordering
        h = (h << 5) | (h >> (sizeof(size_t) * 8 - 5));
        h ^= node.hash();
    }
    return h;
}

ClusterState ClusterState::capture(const std::vector<std::shared_ptr<Node::RaftNode>>& nodes) {
    ClusterState state;
    state.nodes.reserve(nodes.size());
    for (const auto& node : nodes) {
        NodeState ns;
        ns.id = node->id_;
        ns.state = node->get_state();
        ns.term = node->get_term();
        ns.voted_for = node->get_voted_for();
        ns.votes_received = node->get_votes_received();
        state.nodes.push_back(ns);
    }
    return state;
}

} // namespace State
