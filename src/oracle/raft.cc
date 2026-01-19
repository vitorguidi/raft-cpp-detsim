#include "src/oracle/oracle.h"
#include "src/node/node.h"
#include <iostream>

namespace Oracle {

void RaftOracle::enforce_election_safety() {}

void RaftOracle::enforce_invariants() {
    enforce_election_safety();
}

};