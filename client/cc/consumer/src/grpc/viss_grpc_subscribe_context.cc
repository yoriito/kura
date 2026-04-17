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

#include "grpc/viss_grpc_subscribe_context.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>
//
#include "grpc/viss_grpc_inspect_util.h"
#include "grpc/viss_grpc_value_util.h"
#include "yoriito/viss/v1/consumer.grpc.pb.h"
#include "viss_common_error.h"
#include "viss_common_log.h"

namespace yoriito::databroker::consumer {
Result<ConsumerGetResponse> VissGrpcSubscribeContext::Messages() {
  yoriito::viss::v1::consumer::SubscribeCurrentResponse response;

  if (subscribe_response_ == nullptr) {
    VISS_LOG_ERROR("CNSR_SUBSCRIBE_CONTEXT", "No response reader");
    return yoriito::Result<ConsumerGetResponse>::Err(
        VissCommonError::CreateIllegalError("No response reader"));
  }
  bool ret = subscribe_response_->Read(&response);
  std::vector<yoriito::databroker::ConsumerGetDataPoint> vec;
  if (ret) {
    if (response.has_error()) {
      // VISS Error
      auto error = response.error().error().description();
      auto code = response.error().error().number();
      auto message = error + " Code:" + std::to_string(code);
      VISS_LOG_ERROR("CNSR_SUBSCRIBE_CONTEXT", message);

      return yoriito::Result<ConsumerGetResponse>::Err(
          VissCommonError::CreateVissError(code, error));
    }
    if (response.has_event()) {
      // Illegal Response Error Check

      if (response.event().data().data_size() == 0) {
        VISS_LOG_ERROR("CNSR_SUBSCRIBE_CONTEXT", "No data in event");
        return yoriito::Result<yoriito::databroker::ConsumerGetResponse>::Err(
            VissCommonError::CreateIllegalError("No data in event"));
      }

      for (const auto& datapoint : response.event().data().data()) {
        std::string path = datapoint.path();
        for (const auto& dp : datapoint.dp()) {
          ValueType value = VissGrpcValueUtil::ConvertValueType(dp.value());

          int64_t timestamp = dp.storedtimestamp();
          std::optional<int64_t> produced_timestamp = std::nullopt;
          if (dp.has_producedtimestamp()) {
            produced_timestamp = dp.producedtimestamp();
          }
          //
          auto actuator_availability = dp.is_available_actuator();
          auto sensor_availability = dp.is_available_sensor();

          vec.push_back(yoriito::databroker::ConsumerGetDataPoint(
              path, value, sensor_availability, actuator_availability,
              timestamp, produced_timestamp));
        }
      }
    } else {
      // Illegal Response Error Check
      VISS_LOG_ERROR("CNSR_SUBSCRIBE_CONTEXT", "No event in response");
      return yoriito::Result<yoriito::databroker::ConsumerGetResponse>::Err(
          VissCommonError::CreateIllegalError("No event in response"));
    }
  } else {
    auto status = subscribe_response_->Finish();
    if (status.ok()) {
      // Cancel return ok with no data
    } else {
      auto error = status.error_message();
      VISS_LOG_ERROR("CNSR_SUBSCRIBE_CONTEXT", error);
      return yoriito::Result<yoriito::databroker::ConsumerGetResponse>::Err(
          VissCommonError::CreateIllegalError(error));
    }
  }
  return yoriito::Result<yoriito::databroker::ConsumerGetResponse>::Ok(
      yoriito::databroker::ConsumerGetResponse(std::move(vec)));
}

}  // namespace yoriito::databroker::consumer
