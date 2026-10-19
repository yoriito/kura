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

#ifndef PRODUCER_INC_VISS_PRODUCER_IMPL_H_
#define PRODUCER_INC_VISS_PRODUCER_IMPL_H_

#include <memory>
#include <string>

#include "databroker/databroker_producer.h"

namespace yoriito::databroker {
class VissProducerImpl {
 public:
  VissProducerImpl() = default;
  virtual ~VissProducerImpl() = default;

  // Delete copy operations (prevent slicing in polymorphic base class)
  VissProducerImpl(const VissProducerImpl&) = delete;
  VissProducerImpl& operator=(const VissProducerImpl&) = delete;

  // Delete move operations (prevent slicing in polymorphic base class)
  VissProducerImpl(VissProducerImpl&&) = delete;
  VissProducerImpl& operator=(VissProducerImpl&&) = delete;

  virtual Result<void> Connect(std::string target) = 0;

  virtual Result<yoriito::databroker::ProducerGetResponse> Get(
      const yoriito::databroker::ProducerGetRequest& request) = 0;

  virtual Result<yoriito::databroker::ProducerInspectResponse> Inspect(
      const yoriito::databroker::ProducerInspectRequest& request) = 0;

  virtual Result<void> Set(
      const yoriito::databroker::ProducerSetRequest& data) = 0;

  virtual Result<std::unique_ptr<ProducerSubscribeContext>> Subscribe(
      const yoriito::databroker::ProducerSubscribeRequest& data) = 0;

  virtual Result<void> UnSubscribe(
      std::unique_ptr<ProducerSubscribeContext> subscription_id) = 0;
  virtual Result<void> Define(
      const yoriito::databroker::ProducerDefineRequest& data) = 0;

  virtual Result<void> Delete(
      const yoriito::databroker::ProducerDeleteRequest& data) = 0;
};
}  // namespace yoriito::databroker
#endif  // PRODUCER_INC_VISS_PRODUCER_IMPL_H_
