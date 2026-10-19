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

#include "consumer/test/src/test_producer_stub.h"

#include <string>
#include <vector>

#include "common/inc/viss_common_error.h"
#include "consumer/test/src/test_copyable_value_type.h"
#include "databroker/databroker_producer.h"
#include "databroker/databroker_producer_factory.h"

namespace yoriito::databroker::test_stub {
Result<void> ProducerStub::SetData(const std::string &path,
                                   const ValueType &value) {
  return SetData(path, value, true, true);
}

Result<void> ProducerStub::SetData(const std::string &path,
                                   const ValueType &value,
                                   const bool is_available_sensor,
                                   const bool is_available_actuator) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  yoriito::databroker::ProducerSetRequest sensor_value(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              path, value, is_available_sensor, is_available_actuator, 10)});
  return producer->Set(sensor_value);
}
Result<void> ProducerStub::SetData(const std::string &path,
                                   const ValueType &value,
                                   const bool is_available_sensor,
                                   const bool is_available_actuator,
                                   int64_t timestamp) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  yoriito::databroker::ProducerSetRequest sensor_value(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              path, value, is_available_sensor, is_available_actuator,
              timestamp)});
  return producer->Set(sensor_value);
}

Result<ValueType> ProducerStub::GetData(const std::string &path) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  yoriito::databroker::ProducerGetRequest request_data{std::string(path)};
  yoriito::Result<yoriito::databroker::ProducerGetResponse> ret =
      producer->Get(request_data);
  if (!ret.IsOk()) {
    return Result<ValueType>::Err(ret.TakeErr());
  }
  auto data = ret.TakeOk().value();
  auto &data_points = data.GetDataPoints();
  if (data_points.size() != 1) {
    return Result<ValueType>::Err(
        VissCommonError::CreateIllegalError("value type error"));
  }
  const auto &value = data_points.at(0).GetData();
  return Result<ValueType>::Ok(yoriito::CloneValueType(value));
}
}  // namespace yoriito::databroker::test_stub
