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

#include "common/inc/viss_common_error.h"
#include "grpc/viss_grpc_datapoint_util.h"
#include "grpc/viss_grpc_inspect_util.h"
#include "grpc/viss_grpc_value_util.h"
#include "yoriito/yoriito_log.h"

namespace yoriito::databroker::consumer {
Result<ConsumerGetResponse> VissGrpcSubscribeContext::Messages() {
  yoriito::viss::v1::consumer::SubscribeCurrentResponse response;

  if (subscribe_response_ == nullptr) {
    YORIITO_LOG_ERROR("CNSR_SUBSCRIBE_CONTEXT", "subscribe sequence error");
    return yoriito::Result<ConsumerGetResponse>::Err(
        VissCommonError::CreateIllegalError("subscribe sequence error"));
  }
  bool ret = subscribe_response_->Read(&response);
  std::vector<yoriito::databroker::ConsumerGetDataPoint> vec;
  if (ret) {
    if (response.has_error()) {
      // VISS Error
      const std::string& error = response.error().error().description();
      const int32_t code =
          static_cast<int32_t>(response.error().error().number());
      const std::string& message = error + " Code:" + std::to_string(code);
      YORIITO_LOG_ERROR("CNSR_SUBSCRIBE_CONTEXT", message);
      return yoriito::Result<ConsumerGetResponse>::Err(
          VissCommonError::CreateVissError(code, error));
    }
    if (response.has_event()) {
      // Illegal Response Error Check
      if (response.event().data().data_size() == 0) {
        YORIITO_LOG_ERROR("CNSR_SUBSCRIBE_CONTEXT", "No data in event");
        return yoriito::Result<yoriito::databroker::ConsumerGetResponse>::Err(
            VissCommonError::CreateIllegalError("No data in event"));
      }
      const auto& datapoints = response.event().data().data();
      for (const auto& datapoint : datapoints) {
        std::string path = datapoint.path();
        for (const auto& dp : datapoint.dp()) {
          vec.push_back(VissGrpcDataPointUtil::ConvertDataPoint(dp, path));
        }
      }
    } else {
      // Illegal Response Error Check
      YORIITO_LOG_ERROR("CNSR_SUBSCRIBE_CONTEXT", "No event in response");
      return yoriito::Result<yoriito::databroker::ConsumerGetResponse>::Err(
          VissCommonError::CreateIllegalError("No event in response"));
    }
  } else {
    auto status = subscribe_response_->Finish();
    if (status.ok()) {
      // Cancel return ok with no data
    } else {
      auto error = status.error_message();
      YORIITO_LOG_ERROR("CNSR_SUBSCRIBE_CONTEXT", error);
      return yoriito::Result<yoriito::databroker::ConsumerGetResponse>::Err(
          VissCommonError::CreateIllegalError(error));
    }
  }
  return yoriito::Result<yoriito::databroker::ConsumerGetResponse>::Ok(
      yoriito::databroker::ConsumerGetResponse(std::move(vec)));
}

}  // namespace yoriito::databroker::consumer
