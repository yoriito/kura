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

#ifndef KURA_CLIENT_CC_CONSUMER_INC_GRPC_VISS_GRPC_SUBSCRIBE_CONTEXT_H_
#define KURA_CLIENT_CC_CONSUMER_INC_GRPC_VISS_GRPC_SUBSCRIBE_CONTEXT_H_
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

//
#include "databroker/consumer_get_response.h"
#include "databroker/consumer_subscribe_context.h"
//
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.grpc.pb.h"
#include "yoriito/viss/v1/consumer.grpc.pb.h"
#include "yoriito/viss/v1/types/value.grpc.pb.h"
#include "result.h"

//
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

namespace yoriito::databroker::consumer {
class VissGrpcSubscribeContext : public ConsumerSubscribeContext {
 public:
  VissGrpcSubscribeContext(
      std::unique_ptr<grpc::ClientContext>& client_context,
      std::unique_ptr<grpc::ClientReader<
          yoriito::viss::v1::consumer::SubscribeCurrentResponse>>&
          subscribe_response,
      std::string subscription_id)
      : client_context_(std::move(client_context)),
        subscribe_response_(std::move(subscribe_response)),
        subscription_id_(std::move(subscription_id)) {}

  ~VissGrpcSubscribeContext() {
    /*Automatically close the stream if unsubscribe
                                   is forgotten to be called.if Call Cancel , do
       nothing in TryCancel*/

    Cancel();
  }
  Result<ConsumerGetResponse> Messages() override;
  void Cancel() { client_context_->TryCancel(); }
  std::string GetSubscriptionId() { return subscription_id_; }

 private:
  std::unique_ptr<grpc::ClientContext> client_context_;
  std::unique_ptr<
      grpc::ClientReader<yoriito::viss::v1::consumer::SubscribeCurrentResponse>>
      subscribe_response_;
  std::string subscription_id_;
};

}  // namespace yoriito::databroker::consumer
#endif  // KURA_CLIENT_CC_CONSUMER_INC_GRPC_VISS_GRPC_SUBSCRIBE_CONTEXT_H_
