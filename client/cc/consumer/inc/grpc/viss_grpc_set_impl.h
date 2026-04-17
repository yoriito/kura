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

#ifndef KURA_CLIENT_CC_CONSUMER_INC_GRPC_VISS_GRPC_SET_IMPL_H_
#define KURA_CLIENT_CC_CONSUMER_INC_GRPC_VISS_GRPC_SET_IMPL_H_

#include <memory>
#include <utility>
//
#include "databroker/consumer_set_request.h"
#include "grpc/viss_grpc_value_util.h"

//
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.grpc.pb.h"
#include "yoriito/viss/v1/consumer.grpc.pb.h"
#include "yoriito/viss/v1/consumer.pb.h"

using yoriito::viss::v1::consumer::VISS;

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

namespace yoriito::databroker::consumer {
class VissGrpcSetImpl {
 public:
  explicit VissGrpcSetImpl(yoriito::viss::v1::consumer::VISS::Stub *channel)
      : stub_(channel) {}
  ~VissGrpcSetImpl() = default;

  void CreateSetRequest(yoriito::viss::v1::consumer::SetTargetRequest &request,
                        const ConsumerSetRequest &data);

  grpc::Status Execute(ClientContext &context,
                       yoriito::viss::v1::consumer::SetTargetRequest &request,
                       yoriito::viss::v1::consumer::SetTargetResponse &response);

  Result<void> ConvertSetResponse(
      grpc::Status status,
      yoriito::viss::v1::consumer::SetTargetResponse &response,
      const ConsumerSetRequest &data);

 private:
  yoriito::viss::v1::consumer::VISS::Stub *stub_;
};
}  // namespace yoriito::databroker::consumer
#endif  // KURA_CLIENT_CC_CONSUMER_INC_GRPC_VISS_GRPC_SET_IMPL_H_
