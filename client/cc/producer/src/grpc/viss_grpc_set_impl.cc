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

#include "grpc/viss_grpc_set_impl.h"

#include "common/inc/viss_common_error.h"
#include "grpc/viss_grpc_value_util.h"
#include "yoriito/yoriito_log.h"

namespace yoriito::databroker::producer {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

void VissGrpcSetImpl::CreateSetRequest(
    yoriito::viss::v1::producer::SetCurrentRequest &request,
    const ProducerSetRequest &data) const {
  // producer specific logic
  auto now = std::chrono::system_clock::now();
  std::time_t unix_timestamp = std::chrono::system_clock::to_time_t(now);
  for (const auto &data_point : data.GetDataPoints()) {
    yoriito::viss::v1::DataPackagesCurrent *dpks = request.mutable_data();
    yoriito::viss::v1::DataPackageCurrent *dpk_p = dpks->add_data();
    dpk_p->set_path(data_point.GetPath());
    auto dp = dpk_p->add_dp();
    dp->set_is_available_sensor(data_point.IsAvailableSensor());
    dp->set_is_available_actuator(data_point.IsAvailableActuator());
    const auto &result = VissGrpcValueUtil::SetDataPointValue(
        *dp->mutable_value(), data_point.GetData());
    if (result.IsErr()) {
      YORIITO_LOG_ERROR("PRDR_SET",
                        "VissGrpcValueUtil::SetDataPointValue failed");
    }
    // producer specific logic
    //  set timestamps
    if (data_point.GetProducedTimestamp().has_value()) {
      dp->set_producedtimestamp(data_point.GetProducedTimestamp().value());
    }
    dp->set_storedtimestamp(static_cast<int64_t>(unix_timestamp));
  }
  return;
}

grpc::Status VissGrpcSetImpl::Execute(
    ClientContext &context,
    const yoriito::viss::v1::producer::SetCurrentRequest &request,
    yoriito::viss::v1::producer::SetCurrentResponse &response) const {
  return stub_->SetCurrent(&context, request, &response);
}

Result<void> VissGrpcSetImpl::ConvertSetResponse(
    grpc::Status status,
    const yoriito::viss::v1::producer::SetCurrentResponse &response,
    [[maybe_unused]] const ProducerSetRequest &data) const {
  if (status.ok()) {
    if (response.has_error()) {
      // VISS Error
      const std::string &error = response.error().description();
      const int32_t code = static_cast<int32_t>(response.error().number());
      const std::string &message = error + " Code:" + std::to_string(code);
      YORIITO_LOG_ERROR("PRDR_SET", message);
      return Result<void>::Err(VissCommonError::CreateVissError(code, error));
    }

  } else {
    // GRPC Error
    auto error = status.error_message();
    YORIITO_LOG_ERROR("PRDR_SET", error);
    return Result<void>::Err(VissCommonError::CreateProtocolError(error));
  }
  return Result<void>::Ok();
}

}  // namespace yoriito::databroker::producer
