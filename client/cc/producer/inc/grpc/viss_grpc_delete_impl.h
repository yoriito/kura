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

#ifndef KURA_CLIENT_CC_PRODUCER_INC_GRPC_VISS_GRPC_DELETE_IMPL_H_
#define KURA_CLIENT_CC_PRODUCER_INC_GRPC_VISS_GRPC_DELETE_IMPL_H_

#include <memory>
#include <utility>

#include "databroker/producer_delete_request.h"
#include "result.h"
//
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.grpc.pb.h"
#include "yoriito/viss/v1/producer.grpc.pb.h"
#include "yoriito/viss/v1/producer.pb.h"

using yoriito::viss::v1::producer::VISS;

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

namespace yoriito::databroker::producer {
class VissGrpcDeleteImpl {
 public:
  explicit VissGrpcDeleteImpl(yoriito::viss::v1::producer::VISS::Stub* channel)
      : stub_(channel) {}
  ~VissGrpcDeleteImpl() = default;

  void CreateDeleteRequest(yoriito::viss::v1::producer::DeleteRequest& request,
                           const ProducerDeleteRequest& data);
  grpc::Status Execute(ClientContext& context,
                       yoriito::viss::v1::producer::DeleteRequest& request,
                       yoriito::viss::v1::producer::DeleteResponse& response);

  Result<void> ConvertDeleteResponse(
      grpc::Status status,
      yoriito::viss::v1::producer::DeleteResponse& response);

 private:
  yoriito::viss::v1::producer::VISS::Stub* stub_;
};
}  // namespace yoriito::databroker::producer
#endif  // KURA_CLIENT_CC_PRODUCER_INC_GRPC_VISS_GRPC_DELETE_IMPL_H_
