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

#ifndef CONSUMER_INC_GRPC_VISS_CONSUMER_GRPC_IMPL_H_
#define CONSUMER_INC_GRPC_VISS_CONSUMER_GRPC_IMPL_H_
#include <memory>
#include <string>

#include "consumer/inc/viss_consumer_impl.h"
//

#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/consumer.grpc.pb.h"
#include "yoriito/viss/v1/consumer.pb.h"
#include "yoriito/viss/v1/types/value.grpc.pb.h"

namespace yoriito::databroker {

using yoriito::viss::v1::consumer::VISS;

class GrpcConsumerImpl : public VissConsumerImpl {
 public:
  GrpcConsumerImpl() = default;
  ~GrpcConsumerImpl() override = default;
  Result<void> Connect(std::string target) override;
  Result<ConsumerGetResponse> Get(const ConsumerGetRequest& request) override;

  Result<ConsumerInspectResponse> Inspect(
      const ConsumerInspectRequest& request) override;

  Result<void> Set(const ConsumerSetRequest& data) override;

  Result<std::unique_ptr<ConsumerSubscribeContext>> Subscribe(
      const ConsumerSubscribeRequest& data) override;
  Result<void> UnSubscribe(
      std::unique_ptr<ConsumerSubscribeContext> subscription_id) override;

 private:
  std::unique_ptr<yoriito::viss::v1::consumer::VISS::Stub> stub_;
};
}  // namespace yoriito::databroker
#endif  // CONSUMER_INC_GRPC_VISS_CONSUMER_GRPC_IMPL_H_
