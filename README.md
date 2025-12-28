Milestones:

1 - One node requests random sleep DONE
2 - N nodes request random sleep
3 - N nodes choose a target node at random and ping them
    - this demands implementing the server coroutine
    - also requires a rpc listener that spawns a coroutine to handle request
4 - raft leader election
5 - raft log replication + clients


class system
    node[]          2
    scheduler       1
    executor        1
    network         3
    rpc dispatcher  3

class node
    server coroutine    3
    rpc listener        3
    rpc handler         3
