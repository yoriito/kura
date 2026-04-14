#!/usr/bin/env bash
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


THIS_DIR=$(dirname $(readlink -f $0))
PROTO_DIR=$(readlink -f ${THIS_DIR}/../external/yoriito-viss/proto)
OUT_DIR=$(readlink -f ${THIS_DIR}/../client/vshot/vshot)

python3 -m grpc_tools.protoc \
  --proto_path=${PROTO_DIR} \
  --python_out=${OUT_DIR} \
  --grpc_python_out=${OUT_DIR} \
  --mypy_out=${OUT_DIR} \
  --experimental_allow_proto3_optional \
  ${PROTO_DIR}/yoriito/viss/v1/*.proto ${PROTO_DIR}/yoriito/viss/v1/types/*.proto
