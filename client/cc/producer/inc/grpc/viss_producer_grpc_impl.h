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

#ifndef PRODUCER_INC_GRPC_VISS_PRODUCER_GRPC_IMPL_H_
#define PRODUCER_INC_GRPC_VISS_PRODUCER_GRPC_IMPL_H_

#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/producer.grpc.pb.h"
#include "yoriito/viss/v1/types/value.pb.h"
//
#include "producer/inc/viss_producer_impl.h"

namespace yoriito::databroker {

using yoriito::viss::v1::producer::VISS;

class GrpcProducerImpl : public VissProducerImpl {
 public:
  GrpcProducerImpl() = default;
  ~GrpcProducerImpl() override = default;
  Result<void> Connect(std::string target) override;

  Result<yoriito::databroker::ProducerGetResponse> Get(
      const yoriito::databroker::ProducerGetRequest& request) override;

  Result<yoriito::databroker::ProducerInspectResponse> Inspect(
      const yoriito::databroker::ProducerInspectRequest& request) override;

  Result<void> Set(
      const yoriito::databroker::ProducerSetRequest& data) override;

  Result<std::unique_ptr<ProducerSubscribeContext>> Subscribe(
      const yoriito::databroker::ProducerSubscribeRequest& data) override;

  Result<void> UnSubscribe(
      std::unique_ptr<ProducerSubscribeContext> subscription_id) override;

  Result<void> Define(
      const yoriito::databroker::ProducerDefineRequest& data) override;

  Result<void> Delete(
      const yoriito::databroker::ProducerDeleteRequest& data) override;

 private:
  std::unique_ptr<yoriito::viss::v1::producer::VISS::Stub> stub_;
};
}  // namespace yoriito::databroker
#endif  // PRODUCER_INC_GRPC_VISS_PRODUCER_GRPC_IMPL_H_
