// SPDX-License-Identifier: Apache-2.0
//
// Copyright 2026 JASPAR
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "grpc/viss_grpc_datapoint_util.h"

#include "grpc/viss_grpc_value_util.h"

namespace yoriito::databroker::consumer {

yoriito::databroker::ConsumerGetDataPoint
VissGrpcDataPointUtil::ConvertDataPoint(
    const yoriito::viss::v1::DataPointCurrent& datapoint,
    const std::string& path) {
  ValueType value = VissGrpcValueUtil::ConvertValueType(datapoint.value());
  std::optional<int64_t> produced_timestamp = std::nullopt;
  if (datapoint.has_producedtimestamp()) {
    produced_timestamp = std::optional<int64_t>(datapoint.producedtimestamp());
  }
  return yoriito::databroker::ConsumerGetDataPoint(
      path, value, datapoint.is_available_sensor(),
      datapoint.is_available_actuator(), datapoint.storedtimestamp(),
      produced_timestamp);
}

}  // namespace yoriito::databroker::consumer
