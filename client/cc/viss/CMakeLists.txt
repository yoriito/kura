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
project(yoriito_viss)
set(LIBRARY_NAME "yoriito_viss")

include("${CMAKE_CURRENT_LIST_DIR}/../cmake/yoriito_common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../cmake/common.cmake")
set_directory_properties(PROPERTIES
  COMPILE_OPTIONS ""
)

set(PROTO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/yoriito-viss/proto")
set(PROTO_DIR "${PROTO_ROOT}/yoriito/viss/v1")
file(GLOB VISS_TYPE_PROTOS CONFIGURE_DEPENDS "${PROTO_DIR}/types/*.proto")
set(VISS_PROTOS
  "${PROTO_DIR}/common.proto"
  "${PROTO_DIR}/consumer.proto"
  "${PROTO_DIR}/producer.proto"
  ${VISS_TYPE_PROTOS}
)

set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1")
set(VISS_PROTO_SRCS
  "${GENERATED_DIR}/common.pb.cc"
  "${GENERATED_DIR}/consumer.pb.cc"
  "${GENERATED_DIR}/producer.pb.cc"
  "${GENERATED_DIR}/types/error.pb.cc"
  "${GENERATED_DIR}/types/filter.pb.cc"
  "${GENERATED_DIR}/types/value.pb.cc"
)
set(VISS_GRPC_SRCS
  "${GENERATED_DIR}/common.grpc.pb.cc"
  "${GENERATED_DIR}/consumer.grpc.pb.cc"
  "${GENERATED_DIR}/producer.grpc.pb.cc"
  "${GENERATED_DIR}/types/error.grpc.pb.cc"
  "${GENERATED_DIR}/types/filter.grpc.pb.cc"
  "${GENERATED_DIR}/types/value.grpc.pb.cc"
)
set(VISS_GENERATED_HEADERS
  "${GENERATED_DIR}/common.pb.h"
  "${GENERATED_DIR}/common.grpc.pb.h"
  "${GENERATED_DIR}/consumer.pb.h"
  "${GENERATED_DIR}/consumer.grpc.pb.h"
  "${GENERATED_DIR}/producer.pb.h"
  "${GENERATED_DIR}/producer.grpc.pb.h"
  "${GENERATED_DIR}/types/error.pb.h"
  "${GENERATED_DIR}/types/error.grpc.pb.h"
  "${GENERATED_DIR}/types/filter.pb.h"
  "${GENERATED_DIR}/types/filter.grpc.pb.h"
  "${GENERATED_DIR}/types/value.pb.h"
  "${GENERATED_DIR}/types/value.grpc.pb.h"
)
set(VISS_GENERATED_FILES ${VISS_PROTO_SRCS} ${VISS_GRPC_SRCS} ${VISS_GENERATED_HEADERS})

add_custom_command(
  OUTPUT ${VISS_GENERATED_FILES}
  COMMAND ${_PROTOBUF_PROTOC}
  ARGS
    --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
    --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
    -I "${PROTO_ROOT}"
    --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}
    ${VISS_PROTOS}
  DEPENDS ${VISS_PROTOS}
  VERBATIM
)
add_custom_target(yoriito_generate_proto DEPENDS ${VISS_GENERATED_FILES})

add_library(${LIBRARY_NAME} SHARED
  ${VISS_PROTO_SRCS}
  ${VISS_GRPC_SRCS}
)
add_dependencies(${LIBRARY_NAME} yoriito_generate_proto)
target_link_libraries(${LIBRARY_NAME}
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
)

target_include_directories(
  ${LIBRARY_NAME} AFTER
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  PRIVATE
  ${INTERFACE_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# variables. basically you doesn't need to update here
install(TARGETS ${LIBRARY_NAME} EXPORT yoriito_vissTargets DESTINATION ${LIBRARY_INSTALL_DIR})

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/../cmake/yoriito_vissConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/yoriito_vissConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yoriito_viss
)

install(EXPORT yoriito_vissTargets
  FILE yoriito_viss_apiTargets.cmake
  NAMESPACE yoriito_viss::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yoriito_viss
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/yoriito_vissConfig.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/yoriito_viss
)
