#!/bin/bash

# 1. Setup
REF_FILE=$(mktemp)
CURRENT_FILE=$(mktemp)
DIVERGENCE_COUNT=0
TOTAL_RUNS=10000

# 2. Generate Golden Reference
./bazel-bin/src/simulation/simulation > "$REF_FILE"
echo "Starting $TOTAL_RUNS runs to verify determinism..."

# 3. Execution Loop
for ((i=1; i<=TOTAL_RUNS; i++)); do
    ./bazel-bin/src/simulation/simulation > "$CURRENT_FILE"
    
    # Compare files (silent mode)
    if ! diff -q "$REF_FILE" "$CURRENT_FILE" > /dev/null; then
        ((DIVERGENCE_COUNT++))
    fi

    # Progress Update every 500 runs
    if (( i % 500 == 0 )); then
        echo -ne "Progress: $i/$TOTAL_RUNS (Divergences found so far: $DIVERGENCE_COUNT)\r"
    fi
done

# 4. Final Report
echo -e "\n------------------------------------------"
echo "TEST COMPLETE"
echo "Total Runs:        $TOTAL_RUNS"
echo "Total Divergences: $DIVERGENCE_COUNT"
echo "Determinism Rate:  $(( (TOTAL_RUNS - DIVERGENCE_COUNT) * 100 / TOTAL_RUNS ))%"
echo "------------------------------------------"

# Cleanup
rm "$REF_FILE" "$CURRENT_FILE"