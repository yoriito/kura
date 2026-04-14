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

#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.grpc.pb.h"
#include "yoriito/viss/v1/common.pb.h"
#include "yoriito/viss/v1/types/value.grpc.pb.h"
#include "viss_common_error.h"
#include "viss_common_log.h"
//
using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

namespace yoriito::databroker::consumer {

void VissGrpcSetImpl::CreateSetRequest(
    yoriito::viss::v1::consumer::SetTargetRequest &request,
    const ConsumerSetRequest &data) {
  for (const auto &data_point : data.GetDataPoints()) {
    yoriito::viss::v1::DataPackagesTarget *dpks = request.mutable_data();
    yoriito::viss::v1::DataPackageTarget *dpk_p = dpks->add_data();
    dpk_p->set_path(data_point.GetPath());
    auto dp = dpk_p->add_dp();
    VissGrpcValueUtil::SetDataPointValue(*dp->mutable_value(),
                                         data_point.GetData());
  }
  return;
}

grpc::Status VissGrpcSetImpl::Execute(
    ClientContext &context,
    yoriito::viss::v1::consumer::SetTargetRequest &request,
    yoriito::viss::v1::consumer::SetTargetResponse &response) {
  return stub_->SetTarget(&context, request, &response);
}

Result<void> VissGrpcSetImpl::ConvertSetResponse(
    grpc::Status status,
    yoriito::viss::v1::consumer::SetTargetResponse &response,
    const ConsumerSetRequest &data) {
  if (status.ok()) {
    {
      if (response.has_error()) {
        // VISS Error
        auto error = response.error().description();
        auto code = response.error().number();
        auto message = error + " Code:" + std::to_string(code);
        VISS_LOG_ERROR("CNSR_SET", message);

        return Result<void>::Err(VissCommonError::CreateVissError(code, error));
      }
    }
  } else {
    // GRPC Error
    auto error = status.error_message();
    VISS_LOG_ERROR("CNSR_SET", error);
    return Result<void>::Err(VissCommonError::CreateProtocolError(error));
  }
  return Result<void>::Ok();
}

}  // namespace yoriito::databroker::consumer
