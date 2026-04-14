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

#include <string>
#include <utility>
#include <vector>
//
#include "grpc/viss_grpc_get_impl.h"
#include "grpc/viss_grpc_inspect_util.h"
#include "grpc/viss_grpc_value_util.h"
#include "viss_common_error.h"
#include "viss_common_log.h"

//
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.grpc.pb.h"
#include "yoriito/viss/v1/common.pb.h"
#include "yoriito/viss/v1/types/value.grpc.pb.h"
//
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

namespace yoriito::databroker::producer {

void VissGrpcGetImpl::CreateGetRequest(
    yoriito::viss::v1::producer::GetTargetRequest& request,
    const ProducerGetRequest& data) {
  request.set_path(data.GetPath());
  auto get_filter = request.mutable_filter();
  auto path_filter = get_filter->mutable_paths_parameter();
  if (data.GetFilter().has_value()) {
    auto paths = data.GetFilter().value().GetPaths();
    for (const std::string& path : paths) {
      path_filter->add_paths(path);
    }
  }
  return;
}

grpc::Status VissGrpcGetImpl::Execute(
    ClientContext& context,
    yoriito::viss::v1::producer::GetTargetRequest& request,
    yoriito::viss::v1::producer::GetTargetResponse& response) {
  auto ret = stub_->GetTarget(&context, request, &response);
  return ret;
}

Result<ProducerGetResponse> VissGrpcGetImpl::ConvertGetResponse(
    grpc::Status status,
    yoriito::viss::v1::producer::GetTargetResponse& response,
    const ProducerGetRequest& data) {
  std::vector<yoriito::databroker::ProducerGetDataPoint> datapoints;
  if (status.ok()) {
    // VISS Error
    if (response.has_error()) {
      auto error = response.error().description();
      auto code = response.error().number();
      auto message = error + " Code:" + std::to_string(code);
      VISS_LOG_ERROR("PRDR_GET", message);
      return Result<ProducerGetResponse>::Err(
          VissCommonError::CreateVissError(code, error));
    }
    if (!response.has_data()) {
      // Illegal Response: No Data( actually return viss error )
      VISS_LOG_ERROR("PRDR_GET", "Not Found " + data.GetPath());
      return Result<ProducerGetResponse>::Err(
          VissCommonError::CreateNotFoundError(data.GetPath()));
    }

    for (const auto& package : response.data().data()) {
      for (const auto& dp : package.dp()) {
        ValueType value = VissGrpcValueUtil::ConvertValueType(dp.value());
        datapoints.push_back(
            yoriito::databroker::ProducerGetDataPoint(package.path(), value));
      }
    }
  } else {
    auto error = status.error_message();
    VISS_LOG_ERROR("PRDR_GET", error);
    return Result<ProducerGetResponse>::Err(
        VissCommonError::CreateProtocolError(error));
  }
  return Result<ProducerGetResponse>::Ok(
      ProducerGetResponse(std::move(datapoints)));
}
}  // namespace yoriito::databroker::producer
