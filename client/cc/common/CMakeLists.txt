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
project(databroker_common)
set(LIBRARY_NAME "databroker_common")

include("${CMAKE_CURRENT_LIST_DIR}/../cmake/yoriito_common.cmake")

set(LIBRARY_IMPL_NAME ${LIBRARY_NAME})

set(LOCAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false src/*  src/**/*)

include("${CMAKE_CURRENT_LIST_DIR}/../cmake/common.cmake")

add_library(${LIBRARY_IMPL_NAME}
  SHARED
  ${SRC_FILES}
)
target_link_libraries(${LIBRARY_IMPL_NAME}
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    logger
    yoriito_viss
)

target_include_directories(
    ${LIBRARY_IMPL_NAME} AFTER
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    ${INTERFACE_INCLUDE_DIR}
    ${INTERFACE_INCLUDE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# variables. basically you doesn't need to update here
file(GLOB_RECURSE PUBLIC_HEADERS LIST_DIRECTORIES false *.h *.hpp)
# install(FILES ${PUBLIC_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})
install(TARGETS  ${LIBRARY_IMPL_NAME} EXPORT databroker_commonTargets DESTINATION ${LIBRARY_INSTALL_DIR})

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/../cmake/databroker_commonConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/databroker_commonConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/databroker_common
)

install(EXPORT databroker_commonTargets
  FILE databroker_common_apiTargets.cmake
  NAMESPACE databroker_common::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/databroker_common
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/databroker_commonConfig.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/databroker_common
)
