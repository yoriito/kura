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

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <tuple>
#include <vector>

#include "databroker/databroker_producer_factory.h"
#include "producer/test/src/test_consumer_stub.h"
#include "producer/test/src/test_copyable_value_type.h"
#include "yoriito/databroker_type.h"
#include "yoriito/yoriito_log.h"

//
using ::testing::Bool;
using ::testing::Combine;
using ::testing::TestWithParam;
using ::testing::Values;

//
class TestProducerSet : public testing::Test {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer *producer;
};

TEST_F(TestProducerSet, SetInvalidPath) {
  std::string target_path = "Vehicle.not_found_path4";
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, (uint8_t)10,
                                                    false, false, 10)});

  yoriito::Result<void> ret = producer->Set(actuateList);
  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
            error.get()->ErrorCode());
}
//
class TestProducerSetWithParamActuator
    : public TestWithParam<
          std::tuple<std::string, yoriito::test::CopyableValueType>> {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer *producer;
};
INSTANTIATE_TEST_SUITE_P(
    TestProducerSet, TestProducerSetWithParamActuator,
    ::testing::Values(
        // int8_t
        std::make_tuple(
            "Vehicle.Test.ActuatorInt8Value",
            static_cast<int8_t>(std::numeric_limits<int8_t>::min())),
        std::make_tuple("Vehicle.Test.ActuatorInt8Value",
                        static_cast<int8_t>(std::numeric_limits<int8_t>::max() /
                                            2)),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt8Value",
            static_cast<int8_t>(std::numeric_limits<int8_t>::max())),

        // boolean
        std::make_tuple("Vehicle.Test.ActuatorBooleanValue", true),
        std::make_tuple("Vehicle.Test.ActuatorBooleanValue", false),

        // string
        std::make_tuple("Vehicle.Test.ActuatorStringValue", std::string("")),
        std::make_tuple("Vehicle.Test.ActuatorStringValue",
                        std::string("ABCDEFGHIJK")),

        // float
        std::make_tuple("Vehicle.Test.ActuatorFloatValue",
                        static_cast<float>(std::numeric_limits<float>::min())),
        std::make_tuple("Vehicle.Test.ActuatorFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max() /
                                           2)),
        std::make_tuple("Vehicle.Test.ActuatorFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max())),

        // uint8_t
        std::make_tuple(
            "Vehicle.Test.ActuatorUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::min())),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max())),

        // uint16_t
        std::make_tuple(
            "Vehicle.Test.ActuatorUint16Value",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::min())),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint16Value",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint16Value",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max())),

        // int16_t
        std::make_tuple(
            "Vehicle.Test.ActuatorInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::min())),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max())),

        // int32_t
        std::make_tuple(
            "Vehicle.Test.ActuatorInt32Value",
            static_cast<int32_t>(std::numeric_limits<int32_t>::min())),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt32Value",
            static_cast<int32_t>(std::numeric_limits<int32_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt32Value",
            static_cast<int32_t>(std::numeric_limits<int32_t>::max())),

        // int64_t
        std::make_tuple(
            "Vehicle.Test.ActuatorInt64Value",
            static_cast<int64_t>(std::numeric_limits<int64_t>::min())),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt64Value",
            static_cast<int64_t>(std::numeric_limits<int64_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt64Value",
            static_cast<int64_t>(std::numeric_limits<int64_t>::max())),

        // uint32_t
        std::make_tuple(
            "Vehicle.Test.ActuatorUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::min())),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max())),

        // uint64_t
        std::make_tuple(
            "Vehicle.Test.ActuatorUint64Value",
            static_cast<uint64_t>(std::numeric_limits<uint64_t>::min())),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint64Value",
            static_cast<uint64_t>(std::numeric_limits<uint64_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint64Value",
            static_cast<uint64_t>(std::numeric_limits<uint64_t>::max())),

        // double
        std::make_tuple(
            "Vehicle.Test.ActuatorDoubleValue",
            static_cast<double>(std::numeric_limits<double>::min())),
        std::make_tuple("Vehicle.Test.ActuatorDoubleValue",
                        static_cast<double>(std::numeric_limits<double>::max() /
                                            2)),
        std::make_tuple(
            "Vehicle.Test.ActuatorDoubleValue",
            static_cast<double>(std::numeric_limits<double>::max())),

        // bool array
        std::make_tuple("Vehicle.Test.ActuatorBoolArrayValue",
                        std::vector<bool>{}),
        std::make_tuple("Vehicle.Test.ActuatorBoolArrayValue",
                        std::vector<bool>{true, false, true}),

        // string array
        std::make_tuple("Vehicle.Test.ActuatorStringArrayValue",
                        std::vector<std::string>{}),
        std::make_tuple("Vehicle.Test.ActuatorStringArrayValue",
                        std::vector<std::string>{"AAA", "BBB", "CCC"}),

        // int8_t array
        std::make_tuple("Vehicle.Test.ActuatorInt8ArrayValue",
                        std::vector<int8_t>{}),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt8ArrayValue",
            std::vector<int8_t>{
                std::numeric_limits<int8_t>::min(),
                static_cast<int8_t>(std::numeric_limits<int8_t>::max() / 2),
                std::numeric_limits<int8_t>::max()}),

        // int16_t array
        std::make_tuple("Vehicle.Test.ActuatorInt16ArrayValue",
                        std::vector<int16_t>{}),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt16ArrayValue",
            std::vector<int16_t>{
                std::numeric_limits<int16_t>::min(),
                static_cast<int16_t>(std::numeric_limits<int16_t>::max() / 2),
                std::numeric_limits<int16_t>::max()}),

        // int32_t array
        std::make_tuple("Vehicle.Test.ActuatorInt32ArrayValue",
                        std::vector<int32_t>{}),
        std::make_tuple("Vehicle.Test.ActuatorInt32ArrayValue",
                        std::vector<int32_t>{
                            std::numeric_limits<int32_t>::min(),
                            std::numeric_limits<int32_t>::max() / 2,
                            std::numeric_limits<int32_t>::max()}),

        // int64_t array
        std::make_tuple("Vehicle.Test.ActuatorInt64ArrayValue",
                        std::vector<int64_t>{}),
        std::make_tuple("Vehicle.Test.ActuatorInt64ArrayValue",
                        std::vector<int64_t>{
                            std::numeric_limits<int64_t>::min(),
                            std::numeric_limits<int64_t>::max() / 2,
                            std::numeric_limits<int64_t>::max()}),

        // uint8_t array
        std::make_tuple("Vehicle.Test.ActuatorUint8ArrayValue",
                        std::vector<uint8_t>{}),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint8ArrayValue",
            std::vector<uint8_t>{
                std::numeric_limits<uint8_t>::min(),
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2),
                std::numeric_limits<uint8_t>::max()}),

        // uint16_t array
        std::make_tuple("Vehicle.Test.ActuatorUint16ArrayValue",
                        std::vector<uint16_t>{}),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint16ArrayValue",
            std::vector<uint16_t>{
                std::numeric_limits<uint16_t>::min(),
                static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() / 2),
                std::numeric_limits<uint16_t>::max()}),

        // uint32_t array
        std::make_tuple("Vehicle.Test.ActuatorUint32ArrayValue",
                        std::vector<uint32_t>{}),
        std::make_tuple("Vehicle.Test.ActuatorUint32ArrayValue",
                        std::vector<uint32_t>{
                            std::numeric_limits<uint32_t>::min(),
                            std::numeric_limits<uint32_t>::max() / 2,
                            std::numeric_limits<uint32_t>::max()}),

        // uint64_t array
        std::make_tuple("Vehicle.Test.ActuatorUint64ArrayValue",
                        std::vector<uint64_t>{}),
        std::make_tuple("Vehicle.Test.ActuatorUint64ArrayValue",
                        std::vector<uint64_t>{
                            std::numeric_limits<uint64_t>::min(),
                            std::numeric_limits<uint64_t>::max() / 2,
                            std::numeric_limits<uint64_t>::max()}),

        // float array
        std::make_tuple("Vehicle.Test.ActuatorFloatArrayValue",
                        std::vector<float>{}),
        std::make_tuple("Vehicle.Test.ActuatorFloatArrayValue",
                        std::vector<float>{
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::max() / 2,
                            std::numeric_limits<float>::max()}),

        // double array
        std::make_tuple("Vehicle.Test.ActuatorDoubleArrayValue",
                        std::vector<double>{}),
        std::make_tuple("Vehicle.Test.ActuatorDoubleArrayValue",
                        std::vector<double>{
                            std::numeric_limits<double>::min(),
                            std::numeric_limits<double>::max() / 2,
                            std::numeric_limits<double>::max()})));

TEST_P(TestProducerSetWithParamActuator, SetActuatorPath) {
  const auto &params_tuple = GetParam();
  const auto &target_path = std::get<0>(params_tuple);
  const auto &params = std::get<1>(params_tuple);

  yoriito::databroker::test_stub::ConsumerStub stub;
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              target_path, yoriito::CloneValueType(params.value), true, true,
              0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  ASSERT_TRUE(yoriito::ValueTypeEquals(params.value, data));

  // producer
}

//////

class TestProducerSetWithParamSensor
    : public TestWithParam<
          std::tuple<std::string, yoriito::test::CopyableValueType>> {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer *producer;
};

INSTANTIATE_TEST_SUITE_P(
    TestProducerSet, TestProducerSetWithParamSensor,
    ::testing::Values(
        // boolean
        std::make_tuple("Vehicle.Test.SensorBooleanValue", true),
        std::make_tuple("Vehicle.Test.SensorBooleanValue", false),

        // uint16_t
        std::make_tuple(
            "Vehicle.Test.SensorUint16Value",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::min())),
        std::make_tuple(
            "Vehicle.Test.SensorUint16Value",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.SensorUint16Value",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max())),

        // float
        std::make_tuple("Vehicle.Test.SensorFloatValue",
                        static_cast<float>(std::numeric_limits<float>::min())),
        std::make_tuple("Vehicle.Test.SensorFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max() /
                                           2)),
        std::make_tuple("Vehicle.Test.SensorFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max())),

        // string
        std::make_tuple("Vehicle.Test.SensorStringValue", std::string("")),
        std::make_tuple("Vehicle.Test.SensorStringValue",
                        std::string("ABCDEFGHIJK")),

        // int32_t
        std::make_tuple(
            "Vehicle.Test.SensorInt32Value",
            static_cast<int32_t>(std::numeric_limits<int32_t>::min())),
        std::make_tuple(
            "Vehicle.Test.SensorInt32Value",
            static_cast<int32_t>(std::numeric_limits<int32_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.SensorInt32Value",
            static_cast<int32_t>(std::numeric_limits<int32_t>::max())),

        // uint8_t
        std::make_tuple(
            "Vehicle.Test.SensorUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::min())),
        std::make_tuple(
            "Vehicle.Test.SensorUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.SensorUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max())),

        // int16_t
        std::make_tuple(
            "Vehicle.Test.SensorInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::min())),
        std::make_tuple(
            "Vehicle.Test.SensorInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.SensorInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max())),

        // uint32_t
        std::make_tuple(
            "Vehicle.Test.SensorUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::min())),
        std::make_tuple(
            "Vehicle.Test.SensorUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.SensorUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max())),

        // int8_t
        std::make_tuple(
            "Vehicle.Test.SensorInt8Value",
            static_cast<int8_t>(std::numeric_limits<int8_t>::min())),
        std::make_tuple("Vehicle.Test.SensorInt8Value",
                        static_cast<int8_t>(std::numeric_limits<int8_t>::max() /
                                            2)),
        std::make_tuple(
            "Vehicle.Test.SensorInt8Value",
            static_cast<int8_t>(std::numeric_limits<int8_t>::max())),

        // double
        std::make_tuple(
            "Vehicle.Test.SensorDoubleValue",
            static_cast<double>(std::numeric_limits<double>::min())),
        std::make_tuple("Vehicle.Test.SensorDoubleValue",
                        static_cast<double>(std::numeric_limits<double>::max() /
                                            2)),
        std::make_tuple(
            "Vehicle.Test.SensorDoubleValue",
            static_cast<double>(std::numeric_limits<double>::max())),
        // string[]
        std::make_tuple("Vehicle.Test.SensorStringArrayValue",
                        std::vector<std::string>({})),
        std::make_tuple("Vehicle.Test.SensorStringArrayValue",
                        std::vector<std::string>({"AAA", "BBB", "CCC"})),

        // bool[]
        std::make_tuple("Vehicle.Test.SensorBoolArrayValue",
                        std::vector<bool>({})),
        std::make_tuple("Vehicle.Test.SensorBoolArrayValue",
                        std::vector<bool>({true, false, true})),

        // int8_t[]
        std::make_tuple("Vehicle.Test.SensorInt8ArrayValue",
                        std::vector<int8_t>({})),
        std::make_tuple(
            "Vehicle.Test.SensorInt8ArrayValue",
            std::vector<int8_t>({
                std::numeric_limits<int8_t>::min(),
                static_cast<int8_t>(std::numeric_limits<int8_t>::max() / 2),
                std::numeric_limits<int8_t>::max(),
            })),

        // int16_t[]
        std::make_tuple("Vehicle.Test.SensorInt16ArrayValue",
                        std::vector<int16_t>({})),
        std::make_tuple(
            "Vehicle.Test.SensorInt16ArrayValue",
            std::vector<int16_t>({
                std::numeric_limits<int16_t>::min(),
                static_cast<int16_t>(std::numeric_limits<int16_t>::max() / 2),
                std::numeric_limits<int16_t>::max(),
            })),

        // int32_t[]
        std::make_tuple("Vehicle.Test.SensorInt32ArrayValue",
                        std::vector<int32_t>({})),
        std::make_tuple("Vehicle.Test.SensorInt32ArrayValue",
                        std::vector<int32_t>({
                            std::numeric_limits<int32_t>::min(),
                            std::numeric_limits<int32_t>::max() / 2,
                            std::numeric_limits<int32_t>::max(),
                        })),

        // int64_t[]
        std::make_tuple("Vehicle.Test.SensorInt64ArrayValue",
                        std::vector<int64_t>({})),
        std::make_tuple("Vehicle.Test.SensorInt64ArrayValue",
                        std::vector<int64_t>({
                            std::numeric_limits<int64_t>::min(),
                            std::numeric_limits<int64_t>::max() / 2,
                            std::numeric_limits<int64_t>::max(),
                        })),

        // uint8_t[]
        std::make_tuple("Vehicle.Test.SensorUint8ArrayValue",
                        std::vector<uint8_t>({})),
        std::make_tuple(
            "Vehicle.Test.SensorUint8ArrayValue",
            std::vector<uint8_t>({
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::min()),
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2),
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::max()),
            })),

        // uint16_t[]
        std::make_tuple("Vehicle.Test.SensorUint16ArrayValue",
                        std::vector<uint16_t>({})),
        std::make_tuple(
            "Vehicle.Test.SensorUint16ArrayValue",
            std::vector<uint16_t>({
                std::numeric_limits<uint16_t>::min(),
                static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() / 2),
                std::numeric_limits<uint16_t>::max(),
            })),

        // uint32_t[]
        std::make_tuple("Vehicle.Test.SensorUint32ArrayValue",
                        std::vector<uint32_t>({})),
        std::make_tuple("Vehicle.Test.SensorUint32ArrayValue",
                        std::vector<uint32_t>({
                            std::numeric_limits<uint32_t>::min(),
                            std::numeric_limits<uint32_t>::max() / 2,
                            std::numeric_limits<uint32_t>::max(),
                        })),

        // uint64_t[]
        std::make_tuple("Vehicle.Test.SensorUint64ArrayValue",
                        std::vector<uint64_t>({})),
        std::make_tuple("Vehicle.Test.SensorUint64ArrayValue",
                        std::vector<uint64_t>({
                            std::numeric_limits<uint64_t>::min(),
                            std::numeric_limits<uint64_t>::max() / 2,
                            std::numeric_limits<uint64_t>::max(),
                        })),

        // float[]
        std::make_tuple("Vehicle.Test.SensorFloatArrayValue",
                        std::vector<float>({})),
        std::make_tuple("Vehicle.Test.SensorFloatArrayValue",
                        std::vector<float>({
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::max() / 2,
                            std::numeric_limits<float>::max(),
                        })),

        // double[]
        std::make_tuple("Vehicle.Test.SensorDoubleArrayValue",
                        std::vector<double>({})),
        std::make_tuple("Vehicle.Test.SensorDoubleArrayValue",
                        std::vector<double>({
                            std::numeric_limits<double>::min(),
                            std::numeric_limits<double>::max() / 2,
                            std::numeric_limits<double>::max(),
                        }))));

TEST_P(TestProducerSetWithParamSensor, SetSensorPath) {
  const auto &params_tuple = GetParam();
  const auto &target_path = std::get<0>(params_tuple);
  const auto &params = std::get<1>(params_tuple);

  yoriito::databroker::test_stub::ConsumerStub stub;
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              target_path, yoriito::CloneValueType(params.value), true, true,
              0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  ASSERT_TRUE(yoriito::ValueTypeEquals(params.value, data));

  // producer
}

class TestProducerSetWithParamAttribute
    : public TestWithParam<
          std::tuple<std::string, yoriito::test::CopyableValueType>> {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer *producer;
};
INSTANTIATE_TEST_SUITE_P(
    TestProducerSet, TestProducerSetWithParamAttribute,
    ::testing::Values(
        // int8_t
        std::make_tuple(
            "Vehicle.Test.AttributeInt8Value",
            static_cast<int8_t>(std::numeric_limits<int8_t>::min())),
        std::make_tuple("Vehicle.Test.AttributeInt8Value",
                        static_cast<int8_t>(std::numeric_limits<int8_t>::max() /
                                            2)),
        std::make_tuple(
            "Vehicle.Test.AttributeInt8Value",
            static_cast<int8_t>(std::numeric_limits<int8_t>::max())),

        // boolean
        std::make_tuple("Vehicle.Test.AttributeBooleanValue", true),
        std::make_tuple("Vehicle.Test.AttributeBooleanValue", false),

        // string
        std::make_tuple("Vehicle.Test.AttributeStringValue", std::string("")),
        std::make_tuple("Vehicle.Test.AttributeStringValue",
                        std::string("ABCDEFGHIJK")),

        // float
        std::make_tuple("Vehicle.Test.AttributeFloatValue",
                        static_cast<float>(std::numeric_limits<float>::min())),
        std::make_tuple("Vehicle.Test.AttributeFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max() /
                                           2)),
        std::make_tuple("Vehicle.Test.AttributeFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max())),

        // uint8_t
        std::make_tuple(
            "Vehicle.Test.AttributeUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::min())),
        std::make_tuple(
            "Vehicle.Test.AttributeUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.AttributeUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max())),

        // uint16_t
        std::make_tuple(
            "Vehicle.Test.AttributeUint16Value",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::min())),
        std::make_tuple(
            "Vehicle.Test.AttributeUint16Value",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.AttributeUint16Value",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max())),

        // bool[]
        std::make_tuple("Vehicle.Test.AttributeBoolArrayValue",
                        std::vector<bool>({})),
        std::make_tuple("Vehicle.Test.AttributeBoolArrayValue",
                        std::vector<bool>({true, false, true})),

        // string[]
        std::make_tuple("Vehicle.Test.AttributeStringArrayValue",
                        std::vector<std::string>({})),
        std::make_tuple("Vehicle.Test.AttributeStringArrayValue",
                        std::vector<std::string>({"AAA", "BBB", "CCC"})),

        // int8_t[]
        std::make_tuple("Vehicle.Test.AttributeInt8ArrayValue",
                        std::vector<int8_t>({})),
        std::make_tuple(
            "Vehicle.Test.AttributeInt8ArrayValue",
            std::vector<int8_t>({
                std::numeric_limits<int8_t>::min(),
                static_cast<int8_t>(std::numeric_limits<int8_t>::max() / 2),
                std::numeric_limits<int8_t>::max(),
            })),

        // int16_t[]
        std::make_tuple("Vehicle.Test.AttributeInt16ArrayValue",
                        std::vector<int16_t>({})),
        std::make_tuple(
            "Vehicle.Test.AttributeInt16ArrayValue",
            std::vector<int16_t>({
                std::numeric_limits<int16_t>::min(),
                static_cast<int16_t>(std::numeric_limits<int16_t>::max() / 2),
                std::numeric_limits<int16_t>::max(),
            })),

        // int32_t[]
        std::make_tuple("Vehicle.Test.AttributeInt32ArrayValue",
                        std::vector<int32_t>({})),
        std::make_tuple("Vehicle.Test.AttributeInt32ArrayValue",
                        std::vector<int32_t>({
                            std::numeric_limits<int32_t>::min(),
                            std::numeric_limits<int32_t>::max() / 2,
                            std::numeric_limits<int32_t>::max(),
                        })),

        // int64_t[]
        std::make_tuple("Vehicle.Test.AttributeInt64ArrayValue",
                        std::vector<int64_t>({})),
        std::make_tuple("Vehicle.Test.AttributeInt64ArrayValue",
                        std::vector<int64_t>({
                            std::numeric_limits<int64_t>::min(),
                            std::numeric_limits<int64_t>::max() / 2,
                            std::numeric_limits<int64_t>::max(),
                        })),

        // uint8_t[]
        std::make_tuple("Vehicle.Test.AttributeUint8ArrayValue",
                        std::vector<uint8_t>({})),
        std::make_tuple(
            "Vehicle.Test.AttributeUint8ArrayValue",
            std::vector<uint8_t>({
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::min()),
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2),
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::max()),
            })),

        // uint16_t[]
        std::make_tuple("Vehicle.Test.AttributeUint16ArrayValue",
                        std::vector<uint16_t>({})),
        std::make_tuple(
            "Vehicle.Test.AttributeUint16ArrayValue",
            std::vector<uint16_t>({
                std::numeric_limits<uint16_t>::min(),
                static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() / 2),
                std::numeric_limits<uint16_t>::max(),
            })),

        // uint32_t[]
        std::make_tuple("Vehicle.Test.AttributeUint32ArrayValue",
                        std::vector<uint32_t>({})),
        std::make_tuple("Vehicle.Test.AttributeUint32ArrayValue",
                        std::vector<uint32_t>({
                            std::numeric_limits<uint32_t>::min(),
                            std::numeric_limits<uint32_t>::max() / 2,
                            std::numeric_limits<uint32_t>::max(),
                        })),

        // uint64_t[]
        std::make_tuple("Vehicle.Test.AttributeUint64ArrayValue",
                        std::vector<uint64_t>({})),
        std::make_tuple("Vehicle.Test.AttributeUint64ArrayValue",
                        std::vector<uint64_t>({
                            std::numeric_limits<uint64_t>::min(),
                            std::numeric_limits<uint64_t>::max() / 2,
                            std::numeric_limits<uint64_t>::max(),
                        })),

        // float[]
        std::make_tuple("Vehicle.Test.AttributeFloatArrayValue",
                        std::vector<float>({})),
        std::make_tuple("Vehicle.Test.AttributeFloatArrayValue",
                        std::vector<float>({
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::max() / 2,
                            std::numeric_limits<float>::max(),
                        })),

        // double[]
        std::make_tuple("Vehicle.Test.AttributeDoubleArrayValue",
                        std::vector<double>({})),
        std::make_tuple("Vehicle.Test.AttributeDoubleArrayValue",
                        std::vector<double>({
                            std::numeric_limits<double>::min(),
                            std::numeric_limits<double>::max() / 2,
                            std::numeric_limits<double>::max(),
                        }))));

TEST_P(TestProducerSetWithParamAttribute, SetAttributePath) {
  const auto &params_tuple = GetParam();
  const auto &target_path = std::get<0>(params_tuple);
  const auto &params = std::get<1>(params_tuple);

  yoriito::databroker::test_stub::ConsumerStub stub;
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              target_path, yoriito::CloneValueType(params.value), true, true,
              0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  ASSERT_TRUE(yoriito::ValueTypeEquals(params.value, data));

  // producer
}

// set multiple path

TEST_F(TestProducerSet, SetMultiplePathsExistedExisted) {
  yoriito::databroker::test_stub::ConsumerStub stub;
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              "Vehicle.Test.ActuatorBooleanValue", true, true, true, 0),
          yoriito::databroker::ProducerSetDataPoint(
              "Vehicle.Test.ActuatorUint8Value", (uint8_t)255, true, true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  {
    bool target_value = true;
    auto val = stub.GetData("Vehicle.Test.ActuatorBooleanValue");
    ASSERT_TRUE(val.IsOk());
    auto data = val.TakeOk().value();

    auto *p = std::get_if<bool>(&data);
    ASSERT_NE(nullptr, p);
    ASSERT_EQ(target_value, *p);
  }

  {
    uint8_t target_value = 255;
    auto val = stub.GetData("Vehicle.Test.ActuatorUint8Value");
    ASSERT_TRUE(val.IsOk());
    auto data = val.TakeOk().value();

    auto *p = std::get_if<uint8_t>(&data);
    ASSERT_NE(nullptr, p);
    ASSERT_EQ(static_cast<uint8_t>(target_value), *p);
  }
}
TEST_F(TestProducerSet, SetMultiplePathsExistedNotExisted) {
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              "Vehicle.Test.ActuatorUint8Value", (uint8_t)255, true, true, 0),
          yoriito::databroker::ProducerSetDataPoint("Vehicle.not_existed_path",
                                                    true, true, true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
              error.get()->ErrorCode());
  }
}

TEST_F(TestProducerSet, SetPathUnmatchType) {
  std::string target_path = "Vehicle.Test.SensorFloatValue";
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, (int8_t)10,
                                                    true, true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_BAD_REQUEST),
              error.get()->ErrorCode());
  }
}

// SetBranchPath
TEST_F(TestProducerSet, SetBranchPath) {
  std::string target_path = "Vehicle";
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, (uint8_t)10,
                                                    true, true, 0)});

  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    // NOTE:
    // ERROR_BAD_REQUEST may be more better,
    // but to return ERROR_BAD_REQUEST, kura server needs to distinguish just
    // not found and found but it is branch. There doesn't seem to be any
    // problem with integrating these two into ERROR_NOT_FOUND, so I'll do that
    // for now.
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
              error.get()->ErrorCode());
  }
}

TEST_F(TestProducerSet, SetStructAllTypeSignal) {
  const std::string target_path = "Vehicle.Test.StructAllTypeSignal";
  yoriito::databroker::test_stub::ConsumerStub stub;

  auto location_value = std::make_unique<yoriito::StructData>();
  location_value->SetProperty("Latitude", yoriito::ValueType(35.681236));
  location_value->SetProperty("Longitude", yoriito::ValueType(139.767125));
  location_value->SetProperty("Altitude", yoriito::ValueType(12.5));

  auto struct_value = std::make_unique<yoriito::StructData>();
  struct_value->SetProperty("BoolValue", yoriito::ValueType(true));
  struct_value->SetProperty("BoolArrayValue",
                            yoriito::ValueType(std::vector<bool>{true, false}));
  struct_value->SetProperty("StringValue",
                            yoriito::ValueType(std::string("sample")));
  struct_value->SetProperty(
      "StringArrayValue",
      yoriito::ValueType(std::vector<std::string>{"sample", "abc"}));
  struct_value->SetProperty("Int8Value", yoriito::ValueType(int8_t{-12}));
  struct_value->SetProperty("Int8ArrayValue",
                            yoriito::ValueType(std::vector<int8_t>{-12, 34}));
  struct_value->SetProperty("Int16Value", yoriito::ValueType(int16_t{-1234}));
  struct_value->SetProperty(
      "Int16ArrayValue", yoriito::ValueType(std::vector<int16_t>{-1234, 5678}));
  struct_value->SetProperty("Int32Value", yoriito::ValueType(int32_t{-123456}));
  struct_value->SetProperty(
      "Int32ArrayValue",
      yoriito::ValueType(std::vector<int32_t>{-123456, 789012}));
  struct_value->SetProperty("Int64Value",
                            yoriito::ValueType(int64_t{-1234567890}));
  struct_value->SetProperty(
      "Int64ArrayValue",
      yoriito::ValueType(std::vector<int64_t>{-1234567890, 9876543210}));
  struct_value->SetProperty("Uint8Value", yoriito::ValueType(uint8_t{12}));
  struct_value->SetProperty("Uint8ArrayValue",
                            yoriito::ValueType(std::vector<uint8_t>{12, 34}));
  struct_value->SetProperty("Uint16Value", yoriito::ValueType(uint16_t{1234}));
  struct_value->SetProperty(
      "Uint16ArrayValue",
      yoriito::ValueType(std::vector<uint16_t>{1234, 5678}));
  struct_value->SetProperty("Uint32Value",
                            yoriito::ValueType(uint32_t{123456}));
  struct_value->SetProperty(
      "Uint32ArrayValue",
      yoriito::ValueType(std::vector<uint32_t>{123456, 789012}));
  struct_value->SetProperty("Uint64Value",
                            yoriito::ValueType(uint64_t{1234567890}));
  struct_value->SetProperty(
      "Uint64ArrayValue",
      yoriito::ValueType(std::vector<uint64_t>{1234567890, 9876543210}));
  struct_value->SetProperty("FloatValue", yoriito::ValueType(12.5f));
  struct_value->SetProperty(
      "FloatArrayValue", yoriito::ValueType(std::vector<float>{12.5f, 34.5f}));
  struct_value->SetProperty("DoubleValue", yoriito::ValueType(12.5));
  struct_value->SetProperty(
      "DoubleArrayValue", yoriito::ValueType(std::vector<double>{12.5, 34.5}));
  struct_value->SetProperty("LocationValue", std::move(location_value));
  yoriito::test::CopyableValueType target_value(std::move(struct_value));

  yoriito::databroker::ProducerSetRequest request(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              target_path, yoriito::CloneValueType(target_value.value), true,
              true, 0)});
  auto set_ret = producer->Set(request);
  ASSERT_TRUE(set_ret.IsOk());

  auto get_ret = stub.GetData(target_path);
  ASSERT_TRUE(get_ret.IsOk());
  ASSERT_TRUE(
      yoriito::ValueTypeEquals(target_value.value, get_ret.TakeOk().value()));
}

TEST_F(TestProducerSet, SetActuatorStructAllTypeSignal) {
  const std::string target_path = "Vehicle.Test.ActuatorStructAllTypeSignal";
  yoriito::databroker::test_stub::ConsumerStub stub;

  auto location_value = std::make_unique<yoriito::StructData>();
  location_value->SetProperty("Latitude", yoriito::ValueType(35.681236));
  location_value->SetProperty("Longitude", yoriito::ValueType(139.767125));
  location_value->SetProperty("Altitude", yoriito::ValueType(12.5));

  auto struct_value = std::make_unique<yoriito::StructData>();
  struct_value->SetProperty("BoolValue", yoriito::ValueType(true));
  struct_value->SetProperty("BoolArrayValue",
                            yoriito::ValueType(std::vector<bool>{true, false}));
  struct_value->SetProperty("StringValue",
                            yoriito::ValueType(std::string("sample")));
  struct_value->SetProperty(
      "StringArrayValue",
      yoriito::ValueType(std::vector<std::string>{"sample", "abc"}));
  struct_value->SetProperty("Int8Value", yoriito::ValueType(int8_t{-12}));
  struct_value->SetProperty("Int8ArrayValue",
                            yoriito::ValueType(std::vector<int8_t>{-12, 34}));
  struct_value->SetProperty("Int16Value", yoriito::ValueType(int16_t{-1234}));
  struct_value->SetProperty(
      "Int16ArrayValue", yoriito::ValueType(std::vector<int16_t>{-1234, 5678}));
  struct_value->SetProperty("Int32Value", yoriito::ValueType(int32_t{-123456}));
  struct_value->SetProperty(
      "Int32ArrayValue",
      yoriito::ValueType(std::vector<int32_t>{-123456, 789012}));
  struct_value->SetProperty("Int64Value",
                            yoriito::ValueType(int64_t{-1234567890}));
  struct_value->SetProperty(
      "Int64ArrayValue",
      yoriito::ValueType(std::vector<int64_t>{-1234567890, 9876543210}));
  struct_value->SetProperty("Uint8Value", yoriito::ValueType(uint8_t{12}));
  struct_value->SetProperty("Uint8ArrayValue",
                            yoriito::ValueType(std::vector<uint8_t>{12, 34}));
  struct_value->SetProperty("Uint16Value", yoriito::ValueType(uint16_t{1234}));
  struct_value->SetProperty(
      "Uint16ArrayValue",
      yoriito::ValueType(std::vector<uint16_t>{1234, 5678}));
  struct_value->SetProperty("Uint32Value",
                            yoriito::ValueType(uint32_t{123456}));
  struct_value->SetProperty(
      "Uint32ArrayValue",
      yoriito::ValueType(std::vector<uint32_t>{123456, 789012}));
  struct_value->SetProperty("Uint64Value",
                            yoriito::ValueType(uint64_t{1234567890}));
  struct_value->SetProperty(
      "Uint64ArrayValue",
      yoriito::ValueType(std::vector<uint64_t>{1234567890, 9876543210}));
  struct_value->SetProperty("FloatValue", yoriito::ValueType(12.5f));
  struct_value->SetProperty(
      "FloatArrayValue", yoriito::ValueType(std::vector<float>{12.5f, 34.5f}));
  struct_value->SetProperty("DoubleValue", yoriito::ValueType(12.5));
  struct_value->SetProperty(
      "DoubleArrayValue", yoriito::ValueType(std::vector<double>{12.5, 34.5}));
  struct_value->SetProperty("LocationValue", std::move(location_value));
  yoriito::test::CopyableValueType target_value(std::move(struct_value));

  yoriito::databroker::ProducerSetRequest request(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              target_path, yoriito::CloneValueType(target_value.value), true,
              true, 0)});
  auto ret = producer->Set(request);
  ASSERT_TRUE(ret.IsOk());

  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();
  ASSERT_TRUE(yoriito::ValueTypeEquals(data, target_value.value));
}

// SetNestedStructActuatorSignal

TEST_F(TestProducerSet, SetNestedStructActuatorPath) {
  const std::string target_path = "Vehicle.Test.NestedStructSignalActuator";
  yoriito::databroker::test_stub::ConsumerStub stub;

  auto location_value = std::make_unique<yoriito::StructData>();
  location_value->SetProperty("Latitude", yoriito::ValueType(35.681236));
  location_value->SetProperty("Longitude", yoriito::ValueType(139.767125));
  location_value->SetProperty("Altitude", yoriito::ValueType(12.5));

  auto nested_value = std::make_unique<yoriito::StructData>();
  nested_value->SetProperty("property1", yoriito::ValueType(123.45));
  nested_value->SetProperty("property2", std::move(location_value));
  yoriito::test::CopyableValueType target_value(std::move(nested_value));

  yoriito::databroker::ProducerSetRequest request(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              target_path, yoriito::CloneValueType(target_value.value), true,
              true, 0)});

  auto ret = producer->Set(request);
  ASSERT_TRUE(ret.IsOk());

  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();
  ASSERT_TRUE(yoriito::ValueTypeEquals(data, target_value.value));
}

//  SetStructArraySignal

TEST_F(TestProducerSet, SetStructArraySignal) {
  const std::string target_path = "Vehicle.Test.StructArraySignal";
  yoriito::databroker::test_stub::ConsumerStub stub;

  yoriito::StructDataArray struct_array;
  {
    auto location = std::make_unique<yoriito::StructData>();
    location->SetProperty("Latitude", yoriito::ValueType(35.681236));
    location->SetProperty("Longitude", yoriito::ValueType(139.767125));
    location->SetProperty("Altitude", yoriito::ValueType(12.5));
    struct_array.push_back(std::move(location));
  }
  {
    auto location = std::make_unique<yoriito::StructData>();
    location->SetProperty("Latitude", yoriito::ValueType(35.658581));
    location->SetProperty("Longitude", yoriito::ValueType(139.745438));
    location->SetProperty("Altitude", yoriito::ValueType(8.3));
    struct_array.push_back(std::move(location));
  }
  yoriito::test::CopyableValueType target_value(std::move(struct_array));

  yoriito::databroker::ProducerSetRequest request(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              target_path, yoriito::CloneValueType(target_value.value), true,
              true, 0)});

  auto ret = producer->Set(request);
  ASSERT_TRUE(ret.IsOk());

  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();
  ASSERT_TRUE(yoriito::ValueTypeEquals(data, target_value.value));
}

// sensor availability tests
TEST_F(TestProducerSet, SetSensorAvailabilityFalse) {
  std::string target_path = "Vehicle.Test.SensorAvailabilityBooleanValue";
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, (bool)true,
                                                    false, false, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::test_stub::ConsumerStub stub;
    auto val = stub.GetDataPoint(target_path);
    ASSERT_TRUE(val.IsOk());
    const auto data = val.TakeOk().value();
    ASSERT_EQ(false, data.IsAvailableSensor());
    ASSERT_EQ(false, data.IsAvailableActuator());
  }
}
TEST_F(TestProducerSet, SetSensorAvailabilityTrue) {
  std::string target_path = "Vehicle.Test.SensorAvailabilityBooleanValue";
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, (bool)true,
                                                    true, true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::test_stub::ConsumerStub stub;
    auto val = stub.GetDataPoint(target_path);
    ASSERT_TRUE(val.IsOk());
    const auto data = val.TakeOk().value();
    ASSERT_EQ(true, data.IsAvailableSensor());
    ASSERT_EQ(true, data.IsAvailableActuator());
  }
}

TEST_F(TestProducerSet, SetActuatorAvailabilityTrueTrue) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, true, true,
                                                    true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    if (!ret.IsOk()) {
      auto error = ret.TakeErr();
      FAIL() << "Producer set failed with error code: "
             << static_cast<int32_t>(error.get()->ErrorCode());
    }
  }
  {
    yoriito::databroker::test_stub::ConsumerStub stub;
    auto val = stub.GetDataPoint(target_path);
    ASSERT_TRUE(val.IsOk());
    const auto data = val.TakeOk().value();
    ASSERT_EQ(true, data.IsAvailableSensor());
    ASSERT_EQ(true, data.IsAvailableActuator());
  }
}
TEST_F(TestProducerSet, SetActuatorAvailabilityTrueFalse) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, true, true,
                                                    false, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::test_stub::ConsumerStub stub;
    auto val = stub.GetDataPoint(target_path);
    ASSERT_TRUE(val.IsOk());
    const auto data = val.TakeOk().value();
    ASSERT_EQ(true, data.IsAvailableSensor());
    ASSERT_EQ(false, data.IsAvailableActuator());
  }
}
TEST_F(TestProducerSet, SetActuatorAvailabilityFalseTrue) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, true, false,
                                                    true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::test_stub::ConsumerStub stub;
    auto val = stub.GetDataPoint(target_path);
    ASSERT_TRUE(val.IsOk());
    const auto data = val.TakeOk().value();
    ASSERT_EQ(false, data.IsAvailableSensor());
    ASSERT_EQ(true, data.IsAvailableActuator());
  }
}
TEST_F(TestProducerSet, SetActuatorAvailabilityFalseFalse) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, true, false,
                                                    false, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::test_stub::ConsumerStub stub;
    auto val = stub.GetDataPoint(target_path);
    ASSERT_TRUE(val.IsOk());
    const auto data = val.TakeOk().value();
    ASSERT_EQ(false, data.IsAvailableSensor());
    ASSERT_EQ(false, data.IsAvailableActuator());
  }
}

// timestamp tests
TEST_F(TestProducerSet, SetActuatorTimestamp) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";
  int64_t timestamp = 100;
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, true, true,
                                                    true, timestamp)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::test_stub::ConsumerStub stub;
    auto val = stub.GetDataPoint(target_path);
    ASSERT_TRUE(val.IsOk());
    const auto data = val.TakeOk().value();
    ASSERT_EQ(true, data.IsAvailableSensor());
    ASSERT_EQ(true, data.IsAvailableActuator());
    ASSERT_EQ(timestamp, data.GetProducedTimestamp());
  }
}
TEST_F(TestProducerSet, SetSensorTimestamp) {
  std::string target_path = "Vehicle.Test.SensorAvailabilityBooleanValue";
  int64_t timestamp = 100;
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, true, true,
                                                    true, timestamp)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::test_stub::ConsumerStub stub;
    auto val = stub.GetDataPoint(target_path);
    ASSERT_TRUE(val.IsOk());
    const auto data = val.TakeOk().value();
    ASSERT_EQ(true, data.IsAvailableSensor());
    ASSERT_EQ(true, data.IsAvailableActuator());
    ASSERT_EQ(timestamp, data.GetProducedTimestamp());
  }
}
