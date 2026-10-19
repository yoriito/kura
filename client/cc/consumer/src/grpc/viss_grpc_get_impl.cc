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

#include "common/inc/viss_common_error.h"
#include "grpc/viss_grpc_datapoint_util.h"
#include "grpc/viss_grpc_value_util.h"
#include "yoriito/yoriito_log.h"

namespace yoriito::databroker::consumer {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

void VissGrpcGetImpl::CreateGetRequest(
    yoriito::viss::v1::consumer::GetCurrentRequest& request,
    const ConsumerGetRequest& data) const {
  request.set_path(data.GetPath());
  auto get_filter = request.mutable_filter();
  auto path_filter = get_filter->mutable_paths_parameter();
  if (data.GetFilter().has_value()) {
    const auto& paths = data.GetFilter().value().GetPaths();
    for (const std::string& path : paths) {
      path_filter->add_paths(path);
    }
  }
  return;
}

grpc::Status VissGrpcGetImpl::Execute(
    ClientContext& context,
    const yoriito::viss::v1::consumer::GetCurrentRequest& request,
    yoriito::viss::v1::consumer::GetCurrentResponse& response) const {
  auto ret = stub_->GetCurrent(&context, request, &response);
  return ret;
}

Result<ConsumerGetResponse> VissGrpcGetImpl::ConvertGetResponse(
    grpc::Status status,
    const yoriito::viss::v1::consumer::GetCurrentResponse& response,
    const ConsumerGetRequest& data) const {
  std::vector<yoriito::databroker::ConsumerGetDataPoint> datapoints;
  if (status.ok()) {
    // VISS Error
    if (response.has_error()) {
      const std::string& error = response.error().description();
      const int32_t code = static_cast<int32_t>(response.error().number());
      const std::string& message = error + " Code:" + std::to_string(code);
      YORIITO_LOG_ERROR("CNSR_GET", message);
      return Result<ConsumerGetResponse>::Err(
          VissCommonError::CreateVissError(code, error));
    }
    if (!response.has_data()) {
      // Illegal Response: No Data( actually return viss error )
      YORIITO_LOG_ERROR("CNSR_GET", "Not Found " + data.GetPath());
      return Result<ConsumerGetResponse>::Err(
          VissCommonError::CreateNotFoundError(data.GetPath()));
    }

    const auto& packages = response.data().data();
    for (const auto& package : packages) {
      for (const auto& dp : package.dp()) {
        datapoints.push_back(
            VissGrpcDataPointUtil::ConvertDataPoint(dp, package.path()));
      }
    }
  } else {
    auto error = status.error_message();
    YORIITO_LOG_ERROR("CNSR_GET", error);
    return Result<ConsumerGetResponse>::Err(
        VissCommonError::CreateProtocolError(error));
  }
  return Result<ConsumerGetResponse>::Ok(
      ConsumerGetResponse(std::move(datapoints)));
}
}  // namespace yoriito::databroker::consumer
