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
project(databroker_consumer)
set(LIBRARY_NAME "databroker_consumer")

include("${CMAKE_CURRENT_LIST_DIR}/../cmake/yoriito_common.cmake")

set(LIBRARY_IMPL_NAME ${LIBRARY_NAME})

set(LOCAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false src/*  src/**/*)

include("${CMAKE_CURRENT_LIST_DIR}/../cmake/common.cmake")

##########################
# gRPC Generate
##########################
## yoriito_viss
get_filename_component(yoriito_viss_proto    "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/yoriito-viss/proto/yoriito/viss/v1/consumer.proto" ABSOLUTE)
get_filename_component(yoriito_viss_common_proto    "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/yoriito-viss/proto/yoriito/viss/v1/common.proto" ABSOLUTE)
get_filename_component(yoriito_proto_path "${yoriito_viss_proto}" PATH)
get_filename_component(yoriito_viss_proto_root_path    "${CMAKE_CURRENT_SOURCE_DIR}/../../../external/yoriito-viss/proto" ABSOLUTE)

set(yoriito_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/consumer.pb.cc")
set(yoriito_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/consumer.pb.h")
set(yoriito_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/consumer.grpc.pb.cc")
set(yoriito_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/consumer.grpc.pb.h")

set(yoriito_common_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/common.pb.h")
set(yoriito_common_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/common.grpc.pb.h")

set(yoriito_error_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/error.pb.h")
set(yoriito_error_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/error.grpc.pb.h")

set(yoriito_filter_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/filter.pb.h")
set(yoriito_filter_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/filter.grpc.pb.h")

set(yoriito_value_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/value.pb.h")
set(yoriito_value_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/yoriito/viss/v1/types/value.grpc.pb.h")

 file(GLOB yoriito_viss_types_protos "${yoriito_proto_path}/types/*.proto")

add_custom_command(
      OUTPUT "${yoriito_proto_hdrs}" 
            "${yoriito_proto_srcs}" 
            "${yoriito_grpc_hdrs}" 
            "${yoriito_grpc_srcs}" 
            "${yoriito_common_grpc_hdrs}" 
            "${yoriito_common_proto_hdrs}" 
            "${yoriito_error_proto_hdrs}" 
            "${yoriito_error_grpc_hdrs}" 
            "${yoriito_filter_proto_hdrs}" 
            "${yoriito_filter_grpc_hdrs}" 
            "${yoriito_value_proto_hdrs}" 
            "${yoriito_value_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${yoriito_viss_proto_root_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
         "${yoriito_viss_proto}" "${yoriito_viss_common_proto}" ${yoriito_viss_types_protos}
       DEPENDS
         "${yoriito_viss_proto}"
         "${yoriito_viss_common_proto}"
         ${yoriito_viss_types_protos}
)
  
##########################
# gRPC Generate
##########################
include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}"
)
add_custom_target(generate_consumer_dep_protos
     DEPENDS
         ${yoriito_proto_hdrs}
         ${yoriito_grpc_hdrs}
         ${yoriito_common_grpc_hdrs}
         ${yoriito_common_proto_hdrs}
         ${yoriito_error_proto_hdrs}
         ${yoriito_error_grpc_hdrs}
         ${yoriito_filter_proto_hdrs}
         ${yoriito_filter_grpc_hdrs}
         ${yoriito_value_proto_hdrs}
         ${yoriito_value_grpc_hdrs}
)

add_library(${LIBRARY_IMPL_NAME}
  SHARED
  ${SRC_FILES}
## yoriito consumer proto
  ${yoriito_grpc_srcs}
  ${yoriito_proto_srcs}
)

add_dependencies(${LIBRARY_IMPL_NAME} generate_consumer_dep_protos)

set_target_properties(${LIBRARY_IMPL_NAME} PROPERTIES
  INSTALL_RPATH "$ORIGIN"
)

target_link_libraries(${LIBRARY_IMPL_NAME}
  PUBLIC
    databroker_common
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF}
)

target_include_directories(
    ${LIBRARY_IMPL_NAME} AFTER
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
    ${INTERFACE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/inc/
)

if(ENABLE_TESTS)
  add_subdirectory(test)
endif()

# variables. basically you doesn't need to update here
file(GLOB_RECURSE PUBLIC_HEADERS LIST_DIRECTORIES false *.h *.hpp)
# install(FILES ${PUBLIC_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})
install(TARGETS  ${LIBRARY_IMPL_NAME} DESTINATION ${LIBRARY_INSTALL_DIR})
