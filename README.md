# Raft C++ Deterministic Simulator

### Overview
This project is a deterministic discrete-event simulation (DES) framework implemented with C++20 coroutines. The architecture is inspired by FoundationDB’s Flow and Resonate, utilizing coroutines to model distributed systems logic as sequential, readable code while executing within a strictly controlled, single-threaded environment.

### High-Level Architecture

**Components**
* Clock: Virtualized monotonic counter. Time only advances when the simulation loop explicitly ticks.
* RNG: Deterministic random number generator. All simulation entropy is derived from a single seed.
* Executor: A priority queue of tasks sorted by virtual timestamp. It executes all tasks ready at the current virtual time.
* Scheduler: Injects controlled jitter into task timing to simulate non-deterministic execution order within a deterministic framework.
* Network: Simulated wire that buffers and delays messages based on virtual time.
* Router: Scans the network for ready messages, delivers them to node inboxes, and schedules node dispatch tasks.
* Node: The logic container. It uses the System API to perform co_await operations on RPCs and timers.



**The Message Envelope**

Communication between nodes is standardized using an Envelope structure. This contains the routing metadata and a variant for the actual message payload.

```
struct Envelope {
    int message_id, from, to;
    IO::MessageName name;
    IO::Message content;
};
```

**RPC Sequence Diagram**

The following flow describes how a coroutine-based RPC is handled across the system:


1. Node A calls co_await system->rpc().
2. System captures the coroutine_handle, generates a message_id, and pushes an Envelope to the Network.
3. Network calculates an arrival time and stores the message on the wire.
4. Simulation Loop ticks the Clock forward.
5. Router fetches the message when arrival_time <= clock->now() and places it in Node B's inbox.
6. Node B processes the message and sends a response.
7. Router delivers the response back to Node A.
8. System identifies the matching message_id and calls handle.resume() to wake up Node A.



### Execution

**Coroutine Primitives**
Nodes can suspend their execution for either network events (RPC) or time-based events (Sleep).

* RPC (Network Wait): Used to wait for a specific response from a peer. In the PingerNode main loop, the node selects a deterministic target using the RNG and calls co_await system->rpc. The coroutine is suspended, and control returns to the Executor until the corresponding PingResponse is routed back.

* Sleep (Timer Wait): Used to wait for a duration of virtual time. In the SleeperNode main loop, the node draws a random delay and calls co_await system->sleep. The node is suspended and only resumes when the virtual clock reaches the scheduled wake-up time.

**Running Simulations**

```
bazel run //src/simulation:pinger_simulation
bazel run //src/simulation:sleeper_simulation
```

**Determinism Stress Test**

To verify that the simulation is 100% repeatable and free of leakage from the OS (like system time or unseeded randomness), use the stress test script. It runs the simulation 10,000 times and diffs the output of every run against a golden reference.

```
./determinism_stress_test.bash pinger_simulation
````

### Roadmap

* [x] Sleeper Node: Basic timer-based suspension and resumption
* [x] Pinger Nodes and RPC: Full RPC lifecycle with coroutine suspension, network routing, and response-triggered resumption.
* [ ] Raft Leader Election: Implementing Tally-based voting and term management.
* [ ] Raft Log Replication: AppendEntries logic and state machine commitment.
* [ ] Raft Client Requests: External interface for proposing commands to the cluster.
* [ ] Node Restarts: Simulating volatility by clearing volatile state while preserving deterministic recovery.