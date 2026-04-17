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

#include <memory>
#include <string>
#include <utility>
//
#include "grpc/viss_grpc_subscribe_context.h"
#include "grpc/viss_grpc_subscribe_impl.h"
#include "grpc/viss_grpc_subscribe_util.h"
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

void VissGrpcSubscribeImpl::CreateSubscribeRequest(
    yoriito::viss::v1::producer::SubscribeTargetRequest& request,
    const ProducerSubscribeRequest& data) {
  request.set_path(data.GetPath());
  if (data.GetFilter().has_value()) {
    VissGrpcSubscribeUtil::SetFilter(*request.mutable_filter(),
                                     data.GetFilter().value());
  }
  return;
}

std::unique_ptr<
    grpc::ClientReader<yoriito::viss::v1::producer::SubscribeTargetResponse>>
VissGrpcSubscribeImpl::Execute(
    ClientContext& context,
    yoriito::viss::v1::producer::SubscribeTargetRequest& request) {
  return stub_->SubscribeTarget(&context, request);
}

Result<std::unique_ptr<ProducerSubscribeContext>>
VissGrpcSubscribeImpl::ConvertSubscribeResponse(
    std::unique_ptr<grpc::ClientContext>& context,
    std::unique_ptr<
        grpc::ClientReader<yoriito::viss::v1::producer::SubscribeTargetResponse>>
        reader) {
  std::string subscription_id;
  yoriito::viss::v1::producer::SubscribeTargetResponse response;

  bool result = reader->Read(&response);
  if (result) {
    if (response.has_error()) {
      // VISS Error
      auto error = response.error().error().description();
      auto code = response.error().error().number();
      auto message = error + " Code:" + std::to_string(code);
      VISS_LOG_ERROR("PRDR_SUBSCRIBE", message);
      return Result<std::unique_ptr<ProducerSubscribeContext>>::Err(
          VissCommonError::CreateVissError(code, error));
    }
    if (response.has_success()) {
      auto resp = response.success();
      subscription_id = resp.subscription_id();
      std::unique_ptr<ProducerSubscribeContext> ptr =
          std::make_unique<VissGrpcSubscribeContext>(context, reader,
                                                     subscription_id);
      return Result<std::unique_ptr<ProducerSubscribeContext>>::Ok(
          std::move(ptr));
    }
    VISS_LOG_ERROR("PRDR_SUBSCRIBE", "response error");
    return Result<std::unique_ptr<ProducerSubscribeContext>>::Err(
        VissCommonError::CreateIllegalError("response error"));
  } else {
    VISS_LOG_ERROR("PRDR_SUBSCRIBE", "read error");
    return Result<std::unique_ptr<ProducerSubscribeContext>>::Err(
        VissCommonError::CreateIllegalError("read error"));
  }
}

}  // namespace yoriito::databroker::producer
