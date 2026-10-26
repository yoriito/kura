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

# For viss/
option(BUILD_VISS "Enable building viss libraries" ON)

# For common/
option(BUILD_COMMON "Enable building common libraries" ON)

# For consumer/
option(BUILD_CONSUMER "Enable building consumer libraries" ON)
if(BUILD_CONSUMER AND NOT BUILD_COMMON)
    message(FATAL_ERROR
        "BUILD_CONSUMER requires BUILD_COMMON to be ON, but it is currently disabled.")
endif()

# For producer/
option(BUILD_PRODUCER "Enable building producer libraries" ON)
if(BUILD_PRODUCER AND NOT BUILD_COMMON)
    message(FATAL_ERROR
        "BUILD_PRODUCER requires BUILD_COMMON to be ON, but it is currently disabled.")
endif()

# For documentation
option(ENABLE_DOCS "Enable building documentation" OFF)

# For test
option(ENABLE_TESTS "Enable building tests" OFF)

# For fuzz tests (AFL++ only; set AFL_COMPILER=clang (default) or gcc)
option(ENABLE_FUZZ_TESTS "Enable building fuzz harnesses" OFF)
if(ENABLE_TESTS AND (NOT BUILD_CONSUMER AND NOT BUILD_PRODUCER))
    message(FATAL_ERROR
        "ENABLE_TESTS requires at least one of BUILD_CONSUMER or BUILD_PRODUCER to be ON, but both are disabled.")
endif()
if(ENABLE_FUZZ_TESTS AND (NOT BUILD_CONSUMER AND NOT BUILD_COMMON))
    message(FATAL_ERROR
        "ENABLE_FUZZ_TESTS requires BUILD_COMMON and BUILD_CONSUMER to be ON.")
endif()
