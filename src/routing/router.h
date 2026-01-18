#include "src/node/node.h"
#include "src/io/network.h"
#include "src/io/messages.h"
#include <set>

namespace Routing {

class Router {
private:
    std::vector<std::shared_ptr<Node::Node>> nodes_;
    std::shared_ptr<IO::Network> network_;
    std::shared_ptr<System::System> sys_;
public:
    Router(std::vector<std::shared_ptr<Node::Node>> nodes, std::shared_ptr<IO::Network> network, std::shared_ptr<System::System> sys)
        :   nodes_(std::move(nodes)), network_(network), sys_(sys) {}
    void route() {
        std::set<int> ready_nodes;
        for (auto msg : network_->fetch_ready()) {
            nodes_[msg.to]->inbox.push_back(msg);
            ready_nodes.insert(msg.to);
        }
        for (auto node_id : ready_nodes) {
            sys_->request_work([node_id, *this]() {nodes_[node_id]->dispatch();});
        }
    }
};

};