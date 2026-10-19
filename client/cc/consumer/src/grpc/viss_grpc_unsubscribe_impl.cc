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

#include "grpc/viss_grpc_unsubscribe_impl.h"

#include "common/inc/viss_common_error.h"
#include "yoriito/yoriito_log.h"

namespace yoriito::databroker::consumer {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

void VissGrpcUnSubscribeImpl::CreateUnsubscribeRequest(
    const VissGrpcSubscribeContext& viss_context,
    yoriito::viss::v1::UnsubscribeRequest& request) const {
  request.set_subscription_id(viss_context.GetSubscriptionId());
  return;
}

grpc::Status VissGrpcUnSubscribeImpl::Execute(
    ClientContext& context,
    const yoriito::viss::v1::UnsubscribeRequest& request,
    yoriito::viss::v1::UnsubscribeResponse& response) const {
  return stub_->Unsubscribe(&context, request, &response);
}
Result<void> VissGrpcUnSubscribeImpl::ConvertUnsubscribeResponse(
    grpc::Status status,
    const yoriito::viss::v1::UnsubscribeResponse& response) const {
  if (status.ok()) {
    if (response.has_error()) {
      // VISS Error
      const std::string& error = response.error().description();
      const int32_t code = static_cast<int32_t>(response.error().number());
      const std::string& message = error + " Code:" + std::to_string(code);

      YORIITO_LOG_ERROR("CNSR_UNSUBSCRIBE", message);
      return Result<void>::Err(VissCommonError::CreateVissError(code, error));
    }
  } else {
    // GRPC Error
    auto error = status.error_message();
    YORIITO_LOG_ERROR("CNSR_UNSUBSCRIBE", error);
    return Result<void>::Err(VissCommonError::CreateProtocolError(error));
  }
  return Result<void>::Ok();
}
}  // namespace yoriito::databroker::consumer
