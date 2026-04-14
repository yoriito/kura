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

#ifndef KURA_CLIENT_CC_PRODUCER_INC_VISS_PRODUCER_H_
#define KURA_CLIENT_CC_PRODUCER_INC_VISS_PRODUCER_H_

#include <cstdlib>
#include <memory>
#include <string>

#include "databroker/databroker_producer.h"
#include "viss_producer_factory.h"
#include "viss_producer_impl.h"

namespace yoriito::databroker {
class VissProducer : public DataBrokerProducer {
 public:
  static DataBrokerProducer* GetInstance() {
    static VissProducer instance;
    return static_cast<DataBrokerProducer*>(&instance);
  }

  const std::string CONNECTION_INFO = "127.0.0.1:50053";
  VissProducer() {
    impl_ = VissProducerFactory::CreateProducer(
        VissProducerFactory::ProducerType::kGrpc);
    const char* grpc_target = std::getenv("YORIITO_PRODUCER_ENDPOINT");
    if (grpc_target != nullptr && std::string(grpc_target).length() > 0) {
      impl_->Connect(grpc_target);
    } else {
      impl_->Connect(CONNECTION_INFO);
    }
  }
  ~VissProducer() = default;

  Result<yoriito::databroker::ProducerGetResponse> Get(
      const yoriito::databroker::ProducerGetRequest& request) override;

  Result<yoriito::databroker::ProducerInspectResponse> Inspect(
      const yoriito::databroker::ProducerInspectRequest& request) override;

  Result<void> Set(const yoriito::databroker::ProducerSetRequest& data) override;

  Result<std::unique_ptr<ProducerSubscribeContext>> Subscribe(
      const yoriito::databroker::ProducerSubscribeRequest& data) override;

  Result<void> UnSubscribe(
      std::unique_ptr<ProducerSubscribeContext> subscription_id) override;

  Result<void> Define(
      const yoriito::databroker::ProducerDefineRequest& data) override;

  Result<void> Delete(
      const yoriito::databroker::ProducerDeleteRequest& data) override;

 private:
  std::unique_ptr<VissProducerImpl> impl_;
};
}  // namespace yoriito::databroker

#endif  // KURA_CLIENT_CC_PRODUCER_INC_VISS_PRODUCER_H_
