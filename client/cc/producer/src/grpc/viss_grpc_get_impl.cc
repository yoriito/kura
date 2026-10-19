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
#include "grpc/viss_grpc_value_util.h"
#include "yoriito/yoriito_log.h"

namespace yoriito::databroker::producer {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

void VissGrpcGetImpl::CreateGetRequest(
    yoriito::viss::v1::producer::GetTargetRequest& request,
    const ProducerGetRequest& data) const {
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
    const yoriito::viss::v1::producer::GetTargetRequest& request,
    yoriito::viss::v1::producer::GetTargetResponse& response) const {
  auto ret = stub_->GetTarget(&context, request, &response);
  return ret;
}

Result<ProducerGetResponse> VissGrpcGetImpl::ConvertGetResponse(
    grpc::Status status,
    const yoriito::viss::v1::producer::GetTargetResponse& response,
    const ProducerGetRequest& data) const {
  std::vector<yoriito::databroker::ProducerGetDataPoint> datapoints;
  if (status.ok()) {
    // VISS Error
    if (response.has_error()) {
      const std::string& error = response.error().description();
      const int32_t code = static_cast<int32_t>(response.error().number());
      const std::string& message = error + " Code:" + std::to_string(code);
      YORIITO_LOG_ERROR("PRDR_GET", message);
      return Result<ProducerGetResponse>::Err(
          VissCommonError::CreateVissError(code, error));
    }
    if (!response.has_data()) {
      // Illegal Response: No Data( actually return viss error )
      YORIITO_LOG_ERROR("PRDR_GET", "Not Found " + data.GetPath());
      return Result<ProducerGetResponse>::Err(
          VissCommonError::CreateNotFoundError(data.GetPath()));
    }

    const auto& packages = response.data().data();
    for (const auto& package : packages) {
      for (const auto& dp : package.dp()) {
        datapoints.push_back(yoriito::databroker::ProducerGetDataPoint(
            package.path(), VissGrpcValueUtil::ConvertValueType(dp.value())));
      }
    }
  } else {
    auto error = status.error_message();
    YORIITO_LOG_ERROR("PRDR_GET", error);
    return Result<ProducerGetResponse>::Err(
        VissCommonError::CreateProtocolError(error));
  }
  return Result<ProducerGetResponse>::Ok(
      ProducerGetResponse(std::move(datapoints)));
}
}  // namespace yoriito::databroker::producer
