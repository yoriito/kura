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
project(databroker_fuzz)

include("${CMAKE_CURRENT_LIST_DIR}/../cmake/yoriito_common.cmake")

# AFL_COMPILER is derived from CMAKE_CXX_COMPILER automatically.
#
# Build with clang (afl-clang-fast++, LTO instrumentation, faster):
#   cmake -DCMAKE_CXX_COMPILER=afl-clang-fast++ -DENABLE_FUZZ_TESTS=ON ..
#
# Build with gcc (afl-gcc-fast, GCC plugin instrumentation):
#   cmake -DCMAKE_CXX_COMPILER=afl-g++ -DENABLE_FUZZ_TESTS=ON ..
if(CMAKE_CXX_COMPILER MATCHES "afl-clang")
    message(STATUS "Building fuzz harnesses for AFL++ (afl-clang-fast++)")
elseif(CMAKE_CXX_COMPILER MATCHES "afl-g\\+\\+")
    message(STATUS "Building fuzz harnesses for AFL++ (afl-g++)")
else()
    message(WARNING
        "CXX is '${CMAKE_CXX_COMPILER}', which does not look like an AFL++ wrapper. "
        "Set -DCMAKE_CXX_COMPILER=afl-clang-fast++ or afl-g++ for correct instrumentation.")
endif()

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
if(ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
endif()

function(add_fuzz_harness TARGET_NAME SOURCE_FILE)
    cmake_parse_arguments(FUZZ "" "" "LINK_LIBS;INCLUDE_DIRS;PROTO_BINARY_DIRS" ${ARGN})

    add_executable(${TARGET_NAME}
        ${SOURCE_FILE}
    )
    target_include_directories(${TARGET_NAME} PRIVATE
        ${FUZZ_INCLUDE_DIRS}
        ${INTERFACE_INCLUDE_DIR}
        # Generated proto headers live in each library's binary dir.
        ${FUZZ_PROTO_BINARY_DIRS}
    )
    target_link_libraries(${TARGET_NAME} PRIVATE
        ${FUZZ_LINK_LIBS}
    )
endfunction()

get_target_property(_common_binary_dir databroker_common BINARY_DIR)
get_target_property(_consumer_binary_dir databroker_consumer BINARY_DIR)
get_target_property(_producer_binary_dir databroker_producer BINARY_DIR)

add_fuzz_harness(fuzz_convert_value_type
    harness/FuzzConvertValueType.cc
    LINK_LIBS
        databroker_common
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/inc
    PROTO_BINARY_DIRS
        ${_common_binary_dir}
)

add_fuzz_harness(fuzz_convert_get_response
    harness/FuzzConvertGetResponse.cc
    LINK_LIBS
        databroker_consumer
        databroker_common
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../consumer/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/inc
    PROTO_BINARY_DIRS
        ${_consumer_binary_dir}
        ${_common_binary_dir}
)

add_fuzz_harness(fuzz_convert_set_response
    harness/FuzzConvertSetResponse.cc
    LINK_LIBS
        databroker_producer
        databroker_common
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../producer/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/inc
    PROTO_BINARY_DIRS
        ${_producer_binary_dir}
        ${_common_binary_dir}
)

add_fuzz_harness(fuzz_convert_inspect_response
    harness/FuzzConvertInspectResponse.cc
    LINK_LIBS
        databroker_producer
        databroker_common
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/../producer/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/../common/inc
    PROTO_BINARY_DIRS
        ${_producer_binary_dir}
        ${_common_binary_dir}
)
