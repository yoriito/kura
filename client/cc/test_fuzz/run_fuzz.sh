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
export AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1
cd /work/client/cc
export LD_LIBRARY_PATH=./build_fuzz_gcc/common:./build_fuzz_gcc/consumer:./build_fuzz_gcc/producer:$LD_LIBRARY_PATH
afl-fuzz -V "$DURATION" -i test_fuzz/corpus/value_type -o /tmp/findings_value_type -- ./build_fuzz_gcc/test_fuzz/fuzz_convert_value_type
afl-fuzz -V "$DURATION" -i test_fuzz/corpus/empty_value -o /tmp/findings_get_response -- ./build_fuzz_gcc/test_fuzz/fuzz_convert_get_response
afl-fuzz -V "$DURATION" -i test_fuzz/corpus/set_response -o /tmp/findings_set_response -- ./build_fuzz_gcc/test_fuzz/fuzz_convert_set_response
afl-fuzz -V "$DURATION" -i test_fuzz/corpus/inspect_response -o /tmp/findings_inspect_response -- ./build_fuzz_gcc/test_fuzz/fuzz_convert_inspect_response
