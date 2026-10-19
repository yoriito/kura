// SPDX-License-Identifier: Apache-2.0
//
// Copyright 2026 JASPAR
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "consumer/inc/viss_consumer.h"

#include <memory>
#include <string>
#include <utility>
//

namespace yoriito::databroker {
yoriito::Result<ConsumerGetResponse> VissConsumer::Get(
    const ConsumerGetRequest& request) {
  return impl_->Get(request);
}
yoriito::Result<ConsumerInspectResponse> VissConsumer::Inspect(
    const ConsumerInspectRequest& request) {
  return impl_->Inspect(request);
}

yoriito::Result<void> VissConsumer::Set(const ConsumerSetRequest& data) {
  return impl_->Set(data);
}

yoriito::Result<std::unique_ptr<ConsumerSubscribeContext>>
VissConsumer::Subscribe(const ConsumerSubscribeRequest& data) {
  return impl_->Subscribe(data);
}

yoriito::Result<void> VissConsumer::UnSubscribe(
    std::unique_ptr<ConsumerSubscribeContext> subscription_id) {
  return impl_->UnSubscribe(std::move(subscription_id));
}

}  // namespace yoriito::databroker
