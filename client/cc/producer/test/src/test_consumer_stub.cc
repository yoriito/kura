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

#include "test_consumer_stub.h"

#include <string>
#include <vector>

#include "databroker/databroker_consumer.h"
#include "databroker/databroker_consumer_factory.h"

namespace yoriito::databroker::test_stub {

Result<void> ConsumerStub::SetData(const std::string& path, ValueType value) {
  yoriito::databroker::DataBrokerConsumer* Consumer =
      yoriito::databroker::DataBrokerConsumerFactory::GetInstance();

  yoriito::databroker::ConsumerSetRequest sensor_value(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(path, value)});

  return Consumer->Set(sensor_value);
}

Result<ValueType> ConsumerStub::GetData(const std::string& path) {
  yoriito::databroker::DataBrokerConsumer* Consumer =
      yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  yoriito::databroker::ConsumerGetRequest request_data{std::string(path)};
  yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
      Consumer->Get(request_data);
  if (!ret.IsOk()) {
    return Result<ValueType>::Err(ret.TakeErr());
  }
  auto data = ret.TakeOk().value();
  auto& data_points = data.GetDataPoints();
  auto value = data_points.at(0).GetData();
  return Result<ValueType>::Ok(value);
}
}  // namespace yoriito::databroker::test_stub
