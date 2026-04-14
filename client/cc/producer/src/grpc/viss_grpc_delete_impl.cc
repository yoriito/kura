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

#include "grpc/viss_grpc_delete_impl.h"

#include "viss_common_error.h"
#include "viss_common_log.h"
//
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.grpc.pb.h"
#include "yoriito/viss/v1/common.pb.h"
#include "yoriito/viss/v1/types/value.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;
namespace yoriito::databroker::producer {

void VissGrpcDeleteImpl::CreateDeleteRequest(
    yoriito::viss::v1::producer::DeleteRequest& request,
    const ProducerDeleteRequest& data) {
  for (const auto& path : data.GetPaths()) {
    request.add_path(path);
  }
  return;
}

Result<void> VissGrpcDeleteImpl::ConvertDeleteResponse(
    grpc::Status status, yoriito::viss::v1::producer::DeleteResponse& response) {
  if (status.ok()) {
    if (response.has_error()) {
      // VISS Error
      auto error = response.error().description();
      auto code = response.error().number();
      auto message = error + " Code:" + std::to_string(code);
      VISS_LOG_ERROR("PRDR_DELETE", message);
      return Result<void>::Err(VissCommonError::CreateVissError(code, error));
    }

  } else {
    // GRPC Error
    auto error = status.error_message();
    VISS_LOG_ERROR("PRDR_DELETE", error);
    return Result<void>::Err(VissCommonError::CreateProtocolError(error));
  }
  return Result<void>::Ok();
}

grpc::Status VissGrpcDeleteImpl::Execute(
    ClientContext& context, yoriito::viss::v1::producer::DeleteRequest& request,
    yoriito::viss::v1::producer::DeleteResponse& response) {
  return stub_->Delete(&context, request, &response);
}
}  // namespace yoriito::databroker::producer
