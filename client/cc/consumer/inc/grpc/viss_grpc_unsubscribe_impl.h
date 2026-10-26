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

#ifndef CONSUMER_INC_GRPC_VISS_GRPC_UNSUBSCRIBE_IMPL_H_
#define CONSUMER_INC_GRPC_VISS_GRPC_UNSUBSCRIBE_IMPL_H_

#include <memory>
#include <utility>
//
#include "databroker/consumer_subscribe_context.h"
#include "yoriito/result.h"
//
#include "grpc/viss_grpc_subscribe_context.h"
//
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.pb.h"
#include "yoriito/viss/v1/consumer.grpc.pb.h"

namespace yoriito::databroker::consumer {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using yoriito::viss::v1::consumer::VISS;

class VissGrpcUnSubscribeImpl {
 public:
  explicit VissGrpcUnSubscribeImpl(
      yoriito::viss::v1::consumer::VISS::Stub* channel)
      : stub_(channel) {}
  ~VissGrpcUnSubscribeImpl() = default;

  void CreateUnsubscribeRequest(
      const VissGrpcSubscribeContext& viss_context,
      yoriito::viss::v1::UnsubscribeRequest& request) const;

  grpc::Status Execute(ClientContext& context,
                       const yoriito::viss::v1::UnsubscribeRequest& request,
                       yoriito::viss::v1::UnsubscribeResponse& response) const;
  Result<void> ConvertUnsubscribeResponse(
      grpc::Status status,
      const yoriito::viss::v1::UnsubscribeResponse& response) const;

 private:
  yoriito::viss::v1::consumer::VISS::Stub* stub_;
};
}  // namespace yoriito::databroker::consumer
#endif  // CONSUMER_INC_GRPC_VISS_GRPC_UNSUBSCRIBE_IMPL_H_
