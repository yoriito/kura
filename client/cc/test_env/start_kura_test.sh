#!/usr/bin/env bash
# SPDX-License-Identifier: Apache-2.0
#
# Copyright 2026 JASPAR
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set -e

cd "$(dirname "$0")"

# Cleanup function to ensure Kura server is terminated
cleanup() {
    if [[ -n "$KURA_PID" ]] && kill -0 "$KURA_PID" 2>/dev/null; then
        echo "Terminating Kura server (PID: $KURA_PID)..."
        kill "$KURA_PID"
        wait "$KURA_PID" 2>/dev/null || true
    fi
}

# Register cleanup to run on script exit
trap cleanup EXIT

# Start kura server
../../../kura/target/release/kura --config-file-path config.yml > /dev/null 2>&1 &
KURA_PID=$!

# Wait for the server to start
sleep 2

# Extract port numbers from server.json
CONSUMER_PORT=$(grep -A 2 '"ConsumerPortNum"' server.json | grep "default" | grep -oE '[0-9]+' | head -1)
PROVIDER_PORT=$(grep -A 2 '"ProviderPortNum"' server.json | grep "default" | grep -oE '[0-9]+' | head -1)


# Set environment variables and run tests
export YORIITO_CONSUMER_ENDPOINT="127.0.0.1:${CONSUMER_PORT}"
export YORIITO_PRODUCER_ENDPOINT="127.0.0.1:${PROVIDER_PORT}"

# Run the tests and capture exit code
set +e
ctest --test-dir ../build/consumer > /work/quality/assets/unit_test-consumer.md
ctest --test-dir ../build/producer > /work/quality/assets/unit_test-producer.md
TEST_EXIT_CODE=$?
set -e

# Exit with test result (cleanup will be called automatically via trap)
exit $TEST_EXIT_CODE
