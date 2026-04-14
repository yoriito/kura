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

#include "grpc/viss_grpc_get_impl.h"

#include "grpc/viss_grpc_value_util.h"
#include "viss_common_error.h"
#include "viss_common_log.h"

// grpc header
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.pb.h"
#include "yoriito/viss/v1/types/value.grpc.pb.h"
//
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

namespace yoriito::databroker::consumer {

void VissGrpcGetImpl::CreateGetRequest(
    yoriito::viss::v1::consumer::GetCurrentRequest& request,
    const ConsumerGetRequest& data) {
  request.set_path(data.GetPath());
  auto get_filter = request.mutable_filter();
  auto path_filter = get_filter->mutable_paths_parameter();
  if (data.GetFilter().has_value()) {
    for (const std::string& path : data.GetFilter().value().GetPaths()) {
      path_filter->add_paths(path);
    }
  }
  return;
}
grpc::Status VissGrpcGetImpl::Execute(
    ClientContext& context,
    yoriito::viss::v1::consumer::GetCurrentRequest& request,
    yoriito::viss::v1::consumer::GetCurrentResponse& response) {
  auto ret = stub_->GetCurrent(&context, request, &response);
  return ret;
}

Result<ConsumerGetResponse> VissGrpcGetImpl::ConvertGetResponse(
    grpc::Status status,
    yoriito::viss::v1::consumer::GetCurrentResponse& response,
    const ConsumerGetRequest& data) {
  std::vector<yoriito::databroker::ConsumerGetDataPoint> datapoints;
  if (status.ok()) {
    {
      if (response.has_error()) {
        // VISS Error
        auto error = response.error().description();
        auto code = response.error().number();
        auto message = error + " Code:" + std::to_string(code);
        VISS_LOG_ERROR("CNSR_GET", message);
        return Result<ConsumerGetResponse>::Err(
            VissCommonError::CreateVissError(code, error));
      }

      if (!response.has_data()) {
        // Illegal Response: No Data( actually return viss error )
        VISS_LOG_ERROR("CNSR_GET", "Not Found " + data.GetPath());
        return Result<ConsumerGetResponse>::Err(
            VissCommonError::CreateNotFoundError(data.GetPath()));
      }

      for (const auto& package : response.data().data()) {
        for (const auto& dp : package.dp()) {
          ValueType value = VissGrpcValueUtil::ConvertValueType(dp.value());

          int64_t timestamp = dp.storedtimestamp();
          std::optional<int64_t> produced_timestamp = std::nullopt;
          if (dp.has_producedtimestamp()) {
            produced_timestamp = dp.producedtimestamp();
          }

          auto actuator_availability = dp.is_available_actuator();
          auto sensor_availability = dp.is_available_sensor();

          datapoints.push_back(yoriito::databroker::ConsumerGetDataPoint(
              package.path(), value, sensor_availability, actuator_availability,
              timestamp, produced_timestamp));
        }
      }
    }
  } else {
    // GRPC Error
    auto error = status.error_message();
    VISS_LOG_ERROR("CNSR_GET", error);
    return Result<ConsumerGetResponse>::Err(
        VissCommonError::CreateProtocolError(error));
  }
  return Result<ConsumerGetResponse>::Ok(
      ConsumerGetResponse(std::move(datapoints)));
}

}  // namespace yoriito::databroker::consumer
