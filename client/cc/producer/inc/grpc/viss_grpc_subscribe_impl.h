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

#ifndef PRODUCER_INC_GRPC_VISS_GRPC_SUBSCRIBE_IMPL_H_
#define PRODUCER_INC_GRPC_VISS_GRPC_SUBSCRIBE_IMPL_H_

#include <memory>
//
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.pb.h"
#include "yoriito/viss/v1/producer.grpc.pb.h"
//
#include "databroker/producer_subscribe_context.h"
#include "databroker/producer_subscribe_request.h"
#include "yoriito/result.h"

namespace yoriito::databroker::producer {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using yoriito::viss::v1::producer::VISS;

class VissGrpcSubscribeImpl {
 public:
  explicit VissGrpcSubscribeImpl(
      yoriito::viss::v1::producer::VISS::Stub* channel)
      : stub_(channel) {}
  ~VissGrpcSubscribeImpl() = default;

  void CreateSubscribeRequest(
      yoriito::viss::v1::producer::SubscribeTargetRequest& request,
      const ProducerSubscribeRequest& data) const;

  std::unique_ptr<
      grpc::ClientReader<yoriito::viss::v1::producer::SubscribeTargetResponse>>
  Execute(
      ClientContext& context,
      const yoriito::viss::v1::producer::SubscribeTargetRequest& request) const;
  Result<std::unique_ptr<ProducerSubscribeContext>> ConvertSubscribeResponse(
      std::unique_ptr<grpc::ClientContext>& context,
      std::unique_ptr<grpc::ClientReader<
          yoriito::viss::v1::producer::SubscribeTargetResponse>>
          reader,
      const ProducerSubscribeRequest& data) const;

 private:
  yoriito::viss::v1::producer::VISS::Stub* stub_;
};
}  // namespace yoriito::databroker::producer
#endif  // PRODUCER_INC_GRPC_VISS_GRPC_SUBSCRIBE_IMPL_H_
