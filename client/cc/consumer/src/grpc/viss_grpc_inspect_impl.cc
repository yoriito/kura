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
#include "grpc/viss_grpc_inspect_impl.h"
#include "grpc/viss_grpc_inspect_util.h"
#include "grpc/viss_grpc_value_util.h"
#include "yoriito_log.h"
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
using yoriito::viss::v1::consumer::VISS;

namespace yoriito::databroker::consumer {

void VissGrpcInspectImpl::CreateInspectRequest(
    yoriito::viss::v1::InspectRequest& request,
    const ConsumerInspectRequest& data) {
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

grpc::Status VissGrpcInspectImpl::Execute(
    ClientContext& context, yoriito::viss::v1::InspectRequest& request,
    yoriito::viss::v1::InspectResponse& response) {
  return stub_->Inspect(&context, request, &response);
}
Result<ConsumerInspectResponse> VissGrpcInspectImpl::ConvertInspectResponse(
    grpc::Status status, yoriito::viss::v1::InspectResponse& response,
    const ConsumerInspectRequest& request) {
  std::vector<yoriito::databroker::ConsumerInspectData> inspect_data_list;
  if (status.ok()) {
    if (response.has_error()) {
      // VISS Error
      auto error = response.error().description();
      auto code = response.error().number();
      auto message = error + " Code:" + std::to_string(code);
      VISS_LOG_ERROR("CNSR_INSPECT", message);

      return Result<ConsumerInspectResponse>::Err(
          VissCommonError::CreateVissError(code, error));
    }

    // Illegal Response: No Data( actually return viss error )
    if (!response.has_metadata() || response.metadata().entries().empty()) {
      YORIITO_LOG_ERROR("CNSR_INSPECT",
                       "Response OK but response consistency failed.");
      return Result<ConsumerInspectResponse>::Err(
          VissCommonError::CreateIllegalError(
              "Response OK but response consistency failed."));
    }

    for (const auto& metadata_package : response.metadata().entries()) {
      auto ret = VissGrpcInspectUtil::ConvertInspectData(metadata_package);
      if (ret.IsOk()) {
        inspect_data_list.push_back(
            yoriito::databroker::ConsumerInspectData(ret.TakeOk().value()));
      }
    }
  } else {
    // GRPC Error
    auto error = status.error_message();
    VISS_LOG_ERROR("CNSR_INSPECT", error);
    return Result<ConsumerInspectResponse>::Err(
        VissCommonError::CreateProtocolError(error));
  }
  yoriito::databroker::ConsumerInspectResponse inspect_response(
      std::move(inspect_data_list));
  return Result<ConsumerInspectResponse>::Ok(inspect_response);
}
}  // namespace yoriito::databroker::consumer
