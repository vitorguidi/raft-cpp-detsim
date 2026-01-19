!/bin/bash

# 1. Configuration
SIM_PREFIX="//src/simulation:"
TOTAL_RUNS=10000

# Check if simulation name is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <simulation_name>"
    echo "Example: $0 pinger_simulation"
    exit 1
fi

SIM_NAME=$1
TARGET_LABEL="${SIM_PREFIX}${SIM_NAME}"

# 2. Build the target
echo "Building $TARGET_LABEL..."
# Using -c opt for speed, but keep -c dbg if you need to debug divergences with GDB
if ! bazel build "$TARGET_LABEL" -c opt; then
    echo "Build failed. Exiting."
    exit 1
fi

# Construct binary path from target name
# //src/simulation:pinger_simulation -> bazel-bin/src/simulation/pinger_simulation
BINARY_PATH="bazel-bin/src/simulation/${SIM_NAME}"

if [ ! -f "$BINARY_PATH" ]; then
    echo "Error: Could not find binary at $BINARY_PATH"
    exit 1
fi

# 3. Setup Reference
REF_FILE=$(mktemp)
CURRENT_FILE=$(mktemp)
DIVERGENCE_COUNT=0

# Generate Golden Reference (the "Ground Truth")
"$BINARY_PATH" > "$REF_FILE"
echo "Golden reference generated. Starting $TOTAL_RUNS runs..."

# 4. Execution Loop
for ((i=1; i<=TOTAL_RUNS; i++)); do
    "$BINARY_PATH" > "$CURRENT_FILE"
    
    # Compare with reference
    if ! diff -q "$REF_FILE" "$CURRENT_FILE" > /dev/null; then
        ((DIVERGENCE_COUNT++))
        # Save the first divergence for inspection
        if [ "$DIVERGENCE_COUNT" -eq 1 ]; then
            cp "$CURRENT_FILE" "divergence_detected.log"
            echo -e "\n[!] First divergence captured in divergence_detected.log"
        fi
    fi

    # Progress Update
    if (( i % 500 == 0 )); then
        echo -ne "Progress: $i/$TOTAL_RUNS (Divergences: $DIVERGENCE_COUNT)\r"
    fi
done

# 5. Final Report
echo -e "\n------------------------------------------"
echo "SIMULATION:        $SIM_NAME"
echo "Total Runs:        $TOTAL_RUNS"
echo "Total Divergences: $DIVERGENCE_COUNT"

if [ "$DIVERGENCE_COUNT" -eq 0 ]; then
    echo "RESULT:            PASSED (100% Deterministic)"
else
    RATE=$(( (TOTAL_RUNS - DIVERGENCE_COUNT) * 100 / TOTAL_RUNS ))
    echo "RESULT:            FAILED ($RATE% Deterministic)"
fi
echo "------------------------------------------"

# Cleanup
rm "$REF_FILE" "$CURRENT_FILE"