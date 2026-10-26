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

#include "producer/inc/viss_producer.h"
//
#include <memory>
#include <utility>

namespace yoriito::databroker {

Result<yoriito::databroker::ProducerGetResponse> VissProducer::Get(
    const yoriito::databroker::ProducerGetRequest& request) {
  return impl_->Get(request);
}
Result<ProducerInspectResponse> VissProducer::Inspect(
    const yoriito::databroker::ProducerInspectRequest& request) {
  return impl_->Inspect(request);
}

Result<void> VissProducer::Set(
    const yoriito::databroker::ProducerSetRequest& data) {
  return impl_->Set(data);
}

Result<std::unique_ptr<ProducerSubscribeContext>> VissProducer::Subscribe(
    const yoriito::databroker::ProducerSubscribeRequest& data) {
  return impl_->Subscribe(data);
}

Result<void> VissProducer::UnSubscribe(
    std::unique_ptr<ProducerSubscribeContext> subscription_id) {
  return impl_->UnSubscribe(std::move(subscription_id));
}

Result<void> VissProducer::Define(const ProducerDefineRequest& data) {
  return impl_->Define(data);
}
Result<void> VissProducer::Delete(const ProducerDeleteRequest& data) {
  return impl_->Delete(data);
}
}  // namespace yoriito::databroker
