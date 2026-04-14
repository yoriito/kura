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

##########################
# gRPC Generate
##########################
## yoriito_viss
get_filename_component(yoriito_viss_common_proto    "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/yoriito-viss/proto/yoriito/viss/v1/common.proto" ABSOLUTE)
get_filename_component(yoriito_proto_path "${yoriito_viss_common_proto}" PATH)
get_filename_component(yoriito_viss_proto_root_path    "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/yoriito-viss/proto" ABSOLUTE)

set(yoriito_common_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/common.pb.cc")
set(yoriito_common_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/common.pb.h")
set(yoriito_common_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/common.grpc.pb.cc")
set(yoriito_common_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/common.grpc.pb.h")

set(yoriito_error_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/error.pb.cc")
set(yoriito_error_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/error.pb.h")
set(yoriito_error_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/error.grpc.pb.cc")
set(yoriito_error_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/error.grpc.pb.h")

set(yoriito_filter_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/filter.pb.cc")
set(yoriito_filter_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/filter.pb.h")
set(yoriito_filter_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/filter.grpc.pb.cc")
set(yoriito_filter_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/filter.grpc.pb.h")

set(yoriito_value_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/value.pb.cc")
set(yoriito_value_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/value.pb.h")
set(yoriito_value_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/value.grpc.pb.cc")
set(yoriito_value_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/value.grpc.pb.h")


add_custom_command(
      OUTPUT "${yoriito_common_proto_srcs}"	
              "${yoriito_common_grpc_srcs}"	
              "${yoriito_error_proto_srcs}"	
              "${yoriito_error_grpc_srcs}"	
              "${yoriito_filter_grpc_srcs}"	
              "${yoriito_filter_proto_srcs}"	
              "${yoriito_value_grpc_srcs}"	
              "${yoriito_value_proto_srcs}"

      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${yoriito_viss_proto_root_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${yoriito_viss_common_proto}" "${yoriito_proto_path}/types/*.proto"
      DEPENDS "${yoriito_viss_common_proto}"
)

##########################
# gRPC Generate
##########################
include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
)

add_library(${LIBRARY_IMPL_NAME}
  SHARED
  ${SRC_FILES}
## consumer proto
  ${yoriito_common_proto_srcs}
  ${yoriito_common_grpc_srcs}
  ${yoriito_error_grpc_srcs}
  ${yoriito_error_proto_srcs}
  ${yoriito_filter_grpc_srcs}
  ${yoriito_filter_proto_srcs}
  ${yoriito_value_grpc_srcs}
  ${yoriito_value_proto_srcs}
)
target_link_libraries(${LIBRARY_IMPL_NAME}
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
    logger
)

target_include_directories(
    ${LIBRARY_IMPL_NAME} AFTER
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    ${INTERFACE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

# variables. basically you doesn't need to update here
file(GLOB_RECURSE PUBLIC_HEADERS LIST_DIRECTORIES false *.h *.hpp)
# install(FILES ${PUBLIC_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})
install(TARGETS  ${LIBRARY_IMPL_NAME} DESTINATION ${LIBRARY_INSTALL_DIR})
