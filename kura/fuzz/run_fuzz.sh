#!/bin/bash
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
DURATION=${1:-30}
cd "$(dirname "$0")"
for target in fuzz_value_type_conversion fuzz_glob_path_filter fuzz_mem_database_set fuzz_get_condition_glob fuzz_change_filter fuzz_grpc_consumer fuzz_grpc_producer; do
    echo "=== $target ==="
    cargo +nightly fuzz run "$target" -- -max_total_time="$DURATION" || exit 1
done
