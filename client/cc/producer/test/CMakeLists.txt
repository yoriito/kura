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
set(EXECUTABLE_NAME "test_databroker_producer")

include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/yoriito_common.cmake")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

set(LOCAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false src/*  src/**/*)

add_executable(
  ${EXECUTABLE_NAME}
  ${SRC_FILES}
)

target_link_libraries(${EXECUTABLE_NAME}
  GTest::gtest_main
  gmock
  databroker_consumer
  databroker_producer
  logger
)
target_link_directories(${EXECUTABLE_NAME}
  PRIVATE ${INTERFACE_LIBRARY_DIR}
)

target_include_directories(
    ${EXECUTABLE_NAME} AFTER
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    ${INTERFACE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../common/inc
)

include(GoogleTest)
gtest_discover_tests(${EXECUTABLE_NAME})
