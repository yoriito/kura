/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright 2026 JASPAR
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PRODUCER_INC_GRPC_VISS_GRPC_SUBSCRIBE_CONTEXT_H_
#define PRODUCER_INC_GRPC_VISS_GRPC_SUBSCRIBE_CONTEXT_H_

#include <map>
#include <memory>
#include <string>
#include <utility>
//
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.pb.h"
#include "yoriito/viss/v1/producer.grpc.pb.h"
#include "yoriito/viss/v1/types/value.pb.h"

//
#include "databroker/producer_get_response.h"
#include "databroker/producer_subscribe_context.h"
#include "yoriito/result.h"

namespace yoriito::databroker::producer {

using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

class VissGrpcSubscribeContext : public ProducerSubscribeContext {
 public:
  VissGrpcSubscribeContext(
      std::unique_ptr<grpc::ClientContext>& context,
      std::unique_ptr<grpc::ClientReader<
          yoriito::viss::v1::producer::SubscribeTargetResponse>>&
          subscribe_response,
      const std::string& subscription_id)
      : ProducerSubscribeContext(),
        context_(std::move(context)),
        subscribe_response_(std::move(subscribe_response)),
        subscription_id_(subscription_id) {}
  ~VissGrpcSubscribeContext() override {
    /*Automatically close the stream if unsubscribe
                                   is forgotten to be called.if Call Cancel , do
       nothing in TryCancel*/

    Cancel();
  }
  // Delete copy operations (unique_ptr member variables cannot be copied)
  VissGrpcSubscribeContext(const VissGrpcSubscribeContext&) = delete;
  VissGrpcSubscribeContext& operator=(const VissGrpcSubscribeContext&) = delete;

  // Delete move operations (gRPC streaming context should not be moved)
  VissGrpcSubscribeContext(VissGrpcSubscribeContext&&) = delete;
  VissGrpcSubscribeContext& operator=(VissGrpcSubscribeContext&&) = delete;

  Result<ProducerGetResponse> Messages() override;
  void Cancel() const { context_->TryCancel(); }
  std::string GetSubscriptionId() const { return subscription_id_; }

 private:
  std::unique_ptr<grpc::ClientContext> context_;
  std::unique_ptr<
      grpc::ClientReader<yoriito::viss::v1::producer::SubscribeTargetResponse>>
      subscribe_response_;
  std::string subscription_id_;
};

}  // namespace yoriito::databroker::producer

#endif  // PRODUCER_INC_GRPC_VISS_GRPC_SUBSCRIBE_CONTEXT_H_
