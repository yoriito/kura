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

cmake_minimum_required(VERSION 3.28)
# TODO: add PROJECT_VERSION
project(databroker_client)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/yoriito_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/options.cmake")

if(ENABLE_TESTS)
    enable_testing()
endif()

if(BUILD_VISS)
    add_subdirectory(viss)
endif()
if(BUILD_COMMON)
    add_subdirectory(common)
endif()
if(BUILD_CONSUMER)
    add_subdirectory(consumer)
endif()
if(BUILD_PRODUCER)
    add_subdirectory(producer)
endif()
if(ENABLE_DOCS)
    add_subdirectory(docs)
endif()
if(ENABLE_FUZZ_TESTS)
    add_subdirectory(test_fuzz)
endif()
