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

#ifndef CONSUMER_INC_VISS_CONSUMER_IMPL_H_
#define CONSUMER_INC_VISS_CONSUMER_IMPL_H_
#include <memory>
#include <string>

#include "databroker/databroker_consumer.h"

namespace yoriito::databroker {
class VissConsumerImpl {
 public:
  VissConsumerImpl() = default;
  virtual ~VissConsumerImpl() = default;

  // Delete copy operations (prevent slicing in polymorphic base class)
  VissConsumerImpl(const VissConsumerImpl&) = delete;
  VissConsumerImpl& operator=(const VissConsumerImpl&) = delete;

  // Delete move operations (prevent slicing in polymorphic base class)
  VissConsumerImpl(VissConsumerImpl&&) = delete;
  VissConsumerImpl& operator=(VissConsumerImpl&&) = delete;

  virtual Result<void> Connect(std::string target) = 0;
  virtual Result<ConsumerGetResponse> Get(
      const ConsumerGetRequest& request) = 0;
  virtual Result<ConsumerInspectResponse> Inspect(
      const ConsumerInspectRequest& request) = 0;
  virtual Result<void> Set(const ConsumerSetRequest& data) = 0;
  virtual Result<std::unique_ptr<ConsumerSubscribeContext>> Subscribe(
      const ConsumerSubscribeRequest& data) = 0;
  virtual Result<void> UnSubscribe(
      std::unique_ptr<ConsumerSubscribeContext> subscription_id) = 0;
};
}  // namespace yoriito::databroker
#endif  // CONSUMER_INC_VISS_CONSUMER_IMPL_H_
