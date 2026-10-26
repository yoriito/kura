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

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "consumer/test/src/test_copyable_value_type.h"
#include "consumer/test/src/test_producer_stub.h"
#include "databroker/databroker_consumer_factory.h"
#include "yoriito/databroker_type.h"

//
using ::testing::Bool;
using ::testing::Combine;
using ::testing::TestWithParam;
using ::testing::Values;

//
class TestConsumerGet : public testing::Test {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}
  yoriito::databroker::DataBrokerConsumer *consumer;
};

TEST_F(TestConsumerGet, GetInvalidPath) {
  std::string target_path = "Vehicle.not_found_path";

  yoriito::databroker::ConsumerGetRequest request_data{
      std::string(target_path)};
  yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
      consumer->Get(request_data);

  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
            error.get()->ErrorCode());
}

TEST_F(TestConsumerGet, GetNotSetValue) {
  std::string target_path = "Vehicle.Test.ActuatorNotSetBooleanValue";

  yoriito::databroker::ConsumerGetRequest request_data{
      std::string(target_path)};
  yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
      consumer->Get(request_data);

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(0, data.GetDataPoints().size());
}

TEST_F(TestConsumerGet, GetBranchPath) {
  std::string target_path = "Vehicle.Test";

  yoriito::databroker::ConsumerGetRequest request_data{
      std::string(target_path)};
  yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
      consumer->Get(request_data);

  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
            error.get()->ErrorCode());
}
class TestConsumerGetWithParamSensor
    : public TestWithParam<
          std::tuple<std::string, yoriito::test::CopyableValueType>> {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerConsumer *consumer;
};

INSTANTIATE_TEST_SUITE_P(
    TestConsumerGet, TestConsumerGetWithParamSensor,
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

TEST_P(TestConsumerGetWithParamSensor, GetValidPathSensor) {
  const auto &params_tuple = GetParam();
  const auto &target_path = std::get<0>(params_tuple);
  const auto &params = std::get<1>(params_tuple);
  yoriito::databroker::test_stub::ProducerStub stub;
  (void)stub.SetData(target_path, params.value);

  yoriito::databroker::ConsumerGetRequest request_data{
      std::string(target_path)};
  yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
      consumer->Get(request_data);

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  const auto &data_points = data.GetDataPoints();
  ASSERT_EQ(target_path, data_points.at(0).GetPath());
  const auto &val = data_points.at(0).GetData();
  ASSERT_TRUE(yoriito::ValueTypeEquals(params.value, val));
}

class TestConsumerGetWithParamActuator
    : public TestWithParam<
          std::tuple<std::string, yoriito::test::CopyableValueType>> {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerConsumer *consumer;
};
INSTANTIATE_TEST_SUITE_P(
    TestConsumerGet, TestConsumerGetWithParamActuator,
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

TEST_P(TestConsumerGetWithParamActuator, GetValidPathActuator) {
  const auto &params_tuple = GetParam();
  const auto &target_path = std::get<0>(params_tuple);
  const auto &params = std::get<1>(params_tuple);
  yoriito::databroker::test_stub::ProducerStub stub;
  (void)stub.SetData(target_path, yoriito::CloneValueType(params.value));

  yoriito::databroker::ConsumerGetRequest request_data{
      std::string(target_path)};
  yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
      consumer->Get(request_data);

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  const auto &data_points = data.GetDataPoints();
  ASSERT_EQ(target_path, data_points.at(0).GetPath());
  const auto &val = data_points.at(0).GetData();
  ASSERT_TRUE(yoriito::ValueTypeEquals(params.value, val));
}

class TestConsumerGetWithParamAttribute
    : public TestWithParam<
          std::tuple<std::string, yoriito::test::CopyableValueType>> {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerConsumer *consumer;
};

INSTANTIATE_TEST_SUITE_P(
    TestConsumerGet, TestConsumerGetWithParamAttribute,
    ::testing::Values(
        // boolean
        std::make_tuple("Vehicle.Test.AttributeBooleanValue", true),
        std::make_tuple("Vehicle.Test.AttributeBooleanValue", false),

        // string
        std::make_tuple("Vehicle.Test.AttributeStringValue", std::string("")),
        std::make_tuple("Vehicle.Test.AttributeStringValue",
                        std::string("ABCDEFGHIJK")),

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

        // int16_t
        std::make_tuple(
            "Vehicle.Test.AttributeInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::min())),
        std::make_tuple(
            "Vehicle.Test.AttributeInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.AttributeInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max())),

        // int32_t
        std::make_tuple(
            "Vehicle.Test.AttributeInt32Value",
            static_cast<int32_t>(std::numeric_limits<int32_t>::min())),
        std::make_tuple(
            "Vehicle.Test.AttributeInt32Value",
            static_cast<int32_t>(std::numeric_limits<int32_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.AttributeInt32Value",
            static_cast<int32_t>(std::numeric_limits<int32_t>::max())),

        // int64_t
        std::make_tuple(
            "Vehicle.Test.AttributeInt64Value",
            static_cast<int64_t>(std::numeric_limits<int64_t>::min())),
        std::make_tuple(
            "Vehicle.Test.AttributeInt64Value",
            static_cast<int64_t>(std::numeric_limits<int64_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.AttributeInt64Value",
            static_cast<int64_t>(std::numeric_limits<int64_t>::max())),

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

        // uint32_t
        std::make_tuple(
            "Vehicle.Test.AttributeUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::min())),
        std::make_tuple(
            "Vehicle.Test.AttributeUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.AttributeUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max())),

        // uint64_t
        std::make_tuple(
            "Vehicle.Test.AttributeUint64Value",
            static_cast<uint64_t>(std::numeric_limits<uint64_t>::min())),
        std::make_tuple(
            "Vehicle.Test.AttributeUint64Value",
            static_cast<uint64_t>(std::numeric_limits<uint64_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.AttributeUint64Value",
            static_cast<uint64_t>(std::numeric_limits<uint64_t>::max())),

        // float
        std::make_tuple("Vehicle.Test.AttributeFloatValue",
                        static_cast<float>(std::numeric_limits<float>::min())),
        std::make_tuple("Vehicle.Test.AttributeFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max() /
                                           2)),
        std::make_tuple("Vehicle.Test.AttributeFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max())),

        // double
        std::make_tuple(
            "Vehicle.Test.AttributeDoubleValue",
            static_cast<double>(std::numeric_limits<double>::min())),
        std::make_tuple("Vehicle.Test.AttributeDoubleValue",
                        static_cast<double>(std::numeric_limits<double>::max() /
                                            2)),
        std::make_tuple(
            "Vehicle.Test.AttributeDoubleValue",
            static_cast<double>(std::numeric_limits<double>::max())),

        // string
        std::make_tuple("Vehicle.Test.AttributeStringArrayValue",
                        std::vector<std::string>({})),
        std::make_tuple("Vehicle.Test.AttributeStringArrayValue",
                        std::vector<std::string>({"AAA", "BBB", "CCC"})),

        // bool[]
        std::make_tuple("Vehicle.Test.AttributeBoolArrayValue",
                        std::vector<bool>({})),
        std::make_tuple("Vehicle.Test.AttributeBoolArrayValue",
                        std::vector<bool>({true, false, true})),

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

TEST_P(TestConsumerGetWithParamAttribute, GetValidPathAttribute) {
  const auto &params_tuple = GetParam();
  const auto &target_path = std::get<0>(params_tuple);
  const auto &params = std::get<1>(params_tuple);
  yoriito::databroker::test_stub::ProducerStub stub;
  (void)stub.SetData(target_path, yoriito::CloneValueType(params.value));

  yoriito::databroker::ConsumerGetRequest request_data{
      std::string(target_path)};
  yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
      consumer->Get(request_data);

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  const auto &data_points = data.GetDataPoints();
  ASSERT_EQ(target_path, data_points.at(0).GetPath());
  const auto &val = data_points.at(0).GetData();
  ASSERT_TRUE(yoriito::ValueTypeEquals(params.value, val));
}

TEST_F(TestConsumerGet, GetWithFilter) {
  yoriito::databroker::test_stub::ProducerStub stub;
  {
    std::string target_path = "Vehicle.Test.SensorFloatValue";
    float target_value = 300.0f;
    auto ret = stub.SetData(target_path, target_value);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    std::string target_path = "Vehicle.Test.ActuatorFloatValue";
    float target_value = 140.0f;
    auto ret = stub.SetData(target_path, target_value);
    ASSERT_TRUE(ret.IsOk());
  }
  const std::string target_path = "Vehicle.Test";
  yoriito::databroker::ConsumerGetRequest request_data(
      std::string(target_path),
      yoriito::databroker::filter::GetFilter(
          {"SensorFloatValue", "ActuatorFloatValue"}));
  yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
      consumer->Get(request_data);
  auto response = ret.TakeOk().value();
  ASSERT_EQ(2, response.GetDataPoints().size());
  bool float_sensor_found = false;
  bool float_actuator_found = false;
  for (const auto &data_point : response.GetDataPoints()) {
    const auto &data = data_point.GetData();
    if (data_point.GetPath() == "Vehicle.Test.SensorFloatValue") {
      ASSERT_EQ(300.0, std::get<float>(data));
      float_sensor_found = true;
    } else if (data_point.GetPath() == "Vehicle.Test.ActuatorFloatValue") {
      ASSERT_EQ(140, std::get<float>(data));
      float_actuator_found = true;
    }
  }
  ASSERT_TRUE(float_sensor_found);
  ASSERT_TRUE(float_actuator_found);
}
TEST_F(TestConsumerGet, GetWithFilterAsterisk) {
  const std::string target_path = "Vehicle.Test";
  yoriito::databroker::test_stub::ProducerStub stub;
  const std::vector<std::pair<std::string, yoriito::test::CopyableValueType>>
      expected_data = {
          {"Vehicle.Test.ActuatorAvailabilityBooleanValue", false},
          {"Vehicle.Test.ActuatorBooleanValue", true},
          {"Vehicle.Test.ActuatorFloatValue", 140.0f},
          {"Vehicle.Test.ActuatorDoubleValue", 140.0},
          {"Vehicle.Test.ActuatorInt8Value", static_cast<int8_t>(-8)},
          {"Vehicle.Test.ActuatorInt16Value", static_cast<int16_t>(-16)},
          {"Vehicle.Test.ActuatorInt32Value", static_cast<int32_t>(-32)},
          {"Vehicle.Test.ActuatorInt64Value", static_cast<int64_t>(-64)},
          {"Vehicle.Test.ActuatorUint8Value", static_cast<uint8_t>(8)},
          {"Vehicle.Test.ActuatorUint16Value", static_cast<uint16_t>(16)},
          {"Vehicle.Test.ActuatorUint32Value", static_cast<uint32_t>(32)},
          {"Vehicle.Test.ActuatorUint64Value", static_cast<uint64_t>(64)},
          {"Vehicle.Test.ActuatorStringValue", std::string("actuator")},
          {"Vehicle.Test.ActuatorBoolArrayValue",
           std::vector<bool>{true, false}},
          {"Vehicle.Test.ActuatorFloatArrayValue",
           std::vector<float>{1.0f, 2.0f}},
          {"Vehicle.Test.ActuatorDoubleArrayValue",
           std::vector<double>{1.0, 2.0}},
          {"Vehicle.Test.ActuatorInt8ArrayValue", std::vector<int8_t>{-1, 1}},
          {"Vehicle.Test.ActuatorInt16ArrayValue", std::vector<int16_t>{-2, 2}},
          {"Vehicle.Test.ActuatorInt32ArrayValue", std::vector<int32_t>{-3, 3}},
          {"Vehicle.Test.ActuatorInt64ArrayValue", std::vector<int64_t>{-4, 4}},
          {"Vehicle.Test.ActuatorUint8ArrayValue", std::vector<uint8_t>{1, 8}},
          {"Vehicle.Test.ActuatorUint16ArrayValue",
           std::vector<uint16_t>{2, 16}},
          {"Vehicle.Test.ActuatorUint32ArrayValue",
           std::vector<uint32_t>{3, 32}},
          {"Vehicle.Test.ActuatorUint64ArrayValue",
           std::vector<uint64_t>{4, 64}},
          {"Vehicle.Test.ActuatorStringArrayValue",
           std::vector<std::string>{"one", "two"}},
      };
  for (const auto &expected : expected_data) {
    ASSERT_TRUE(stub.SetData(expected.first, expected.second.value).IsOk());
  }

  yoriito::databroker::ConsumerGetRequest request_data(
      std::string(target_path),
      yoriito::databroker::filter::GetFilter({"Actuator*Value"}));

  yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
      consumer->Get(request_data);

  ASSERT_TRUE(ret.IsOk());
  auto response = ret.TakeOk().value();
  for (const auto &data_point : response.GetDataPoints()) {
    const auto expected =
        std::find_if(expected_data.begin(), expected_data.end(),
                     [&data_point](const auto &entry) {
                       return entry.first == data_point.GetPath();
                     });
    ASSERT_NE(expected, expected_data.end())
        << "Unexpected path: " << data_point.GetPath();
    ASSERT_TRUE(
        yoriito::ValueTypeEquals(expected->second.value, data_point.GetData()));
  }
}

TEST_F(TestConsumerGet, GetWithFilterInvalidGlob) {
  std::string target_path = "Vehicle.Test";
  yoriito::databroker::ConsumerGetRequest request_data(
      std::string(target_path),
      yoriito::databroker::filter::GetFilter({"[invalid"}));

  auto ret = consumer->Get(request_data);

  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_BAD_REQUEST),
            error.get()->ErrorCode());
}

// strucct data test
TEST_F(TestConsumerGet, GetStructArraySignal) {
  yoriito::databroker::test_stub::ProducerStub stub;
  const std::string target_path = "Vehicle.Test.StructArraySignal";

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

  auto set_ret =
      stub.SetData(target_path, yoriito::CloneValueType(target_value.value));
  ASSERT_TRUE(set_ret.IsOk());

  yoriito::databroker::ConsumerGetRequest request_data{
      std::string(target_path)};
  yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
      consumer->Get(request_data);

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  const auto &data_points = data.GetDataPoints();
  ASSERT_EQ(target_path, data_points.at(0).GetPath());
  const auto &val = data_points.at(0).GetData();
  ASSERT_TRUE(yoriito::ValueTypeEquals(val, target_value.value));
}
TEST_F(TestConsumerGet, GetActuatorStructAllTypeSignal) {
  const std::string target_path = "Vehicle.Test.ActuatorStructAllTypeSignal";
  yoriito::databroker::test_stub::ProducerStub stub;

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

  auto set_ret =
      stub.SetData(target_path, yoriito::CloneValueType(target_value.value));
  ASSERT_TRUE(set_ret.IsOk());

  yoriito::databroker::ConsumerGetRequest request_data{
      std::string(target_path)};
  auto ret = consumer->Get(request_data);

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  const auto &data_points = data.GetDataPoints();
  ASSERT_EQ(target_path, data_points.at(0).GetPath());
  ASSERT_TRUE(yoriito::ValueTypeEquals(target_value.value,
                                       data_points.at(0).GetData()));
}

TEST_F(TestConsumerGet, GetNestedStructActuatorSignal) {
  yoriito::databroker::test_stub::ProducerStub stub;

  const std::string target_path = "Vehicle.Test.NestedStructSignalActuator";

  auto location_value = std::make_unique<yoriito::StructData>();
  location_value->SetProperty("Latitude", yoriito::ValueType(35.681236));
  location_value->SetProperty("Longitude", yoriito::ValueType(139.767125));
  location_value->SetProperty("Altitude", yoriito::ValueType(12.5));

  auto nested_value = std::make_unique<yoriito::StructData>();
  nested_value->SetProperty("property1", yoriito::ValueType(123.45));
  nested_value->SetProperty("property2", std::move(location_value));
  yoriito::test::CopyableValueType target_value(std::move(nested_value));

  auto set_ret =
      stub.SetData(target_path, yoriito::CloneValueType(target_value.value));
  ASSERT_TRUE(set_ret.IsOk());

  yoriito::databroker::ConsumerGetRequest request_data{
      std::string(target_path)};
  auto ret = consumer->Get(request_data);

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  const auto &data_points = data.GetDataPoints();
  ASSERT_EQ(target_path, data_points.at(0).GetPath());
  ASSERT_TRUE(yoriito::ValueTypeEquals(target_value.value,
                                       data_points.at(0).GetData()));
}

// Sensor availability tests
TEST_F(TestConsumerGet, GetSensorAvailabilityFalse) {
  std::string target_path = "Vehicle.Test.SensorAvailabilityBooleanValue";
  {
    bool target_value = false;
    yoriito::databroker::test_stub::ProducerStub stub;
    auto ret = stub.SetData(target_path, target_value, false, false);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::ConsumerGetRequest request_data{
        std::string(target_path)};
    yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
        consumer->Get(request_data);
    ASSERT_TRUE(ret.IsOk());
    auto response = ret.TakeOk().value();
    ASSERT_EQ(1, response.GetDataPoints().size());
    const auto &data_points = response.GetDataPoints();
    ASSERT_EQ(target_path, data_points.at(0).GetPath());
    const auto &val = data_points.at(0).GetData();
    ASSERT_TRUE(yoriito::ValueTypeEquals(false, val));
    bool is_available = data_points.at(0).IsAvailableSensor();
    ASSERT_FALSE(is_available);
  }
}
TEST_F(TestConsumerGet, GetSensorAvailabilityTrue) {
  std::string target_path = "Vehicle.Test.SensorAvailabilityBooleanValue";
  {
    bool target_value = true;
    yoriito::databroker::test_stub::ProducerStub stub;
    auto ret = stub.SetData(target_path, target_value, true, true);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::ConsumerGetRequest request_data{
        std::string(target_path)};
    yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
        consumer->Get(request_data);
    ASSERT_TRUE(ret.IsOk());
    auto response = ret.TakeOk().value();
    ASSERT_EQ(1, response.GetDataPoints().size());
    const auto &data_points = response.GetDataPoints();
    ASSERT_EQ(target_path, data_points.at(0).GetPath());
    const auto &val = data_points.at(0).GetData();
    ASSERT_TRUE(yoriito::ValueTypeEquals(true, val));
    bool is_available = data_points.at(0).IsAvailableSensor();
    ASSERT_TRUE(is_available);
  }
}
// actuator availability tests
TEST_F(TestConsumerGet, GetActuatorAvailabilityFalse) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";
  {
    bool target_value = false;
    yoriito::databroker::test_stub::ProducerStub stub;
    auto ret = stub.SetData(target_path, target_value, false, false);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::ConsumerGetRequest request_data{
        std::string(target_path)};
    yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
        consumer->Get(request_data);
    ASSERT_TRUE(ret.IsOk());
    auto response = ret.TakeOk().value();
    ASSERT_EQ(1, response.GetDataPoints().size());
    const auto &data_points = response.GetDataPoints();
    ASSERT_EQ(target_path, data_points.at(0).GetPath());
    const auto &val = data_points.at(0).GetData();
    ASSERT_TRUE(yoriito::ValueTypeEquals(false, val));
    bool is_available_sensor = data_points.at(0).IsAvailableSensor();
    ASSERT_FALSE(is_available_sensor);
    bool is_available_actuator = data_points.at(0).IsAvailableActuator();
    ASSERT_FALSE(is_available_actuator);
  }
}
TEST_F(TestConsumerGet, GetActuatorAvailabilityTrue) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";
  {
    bool target_value = true;
    yoriito::databroker::test_stub::ProducerStub stub;
    auto ret = stub.SetData(target_path, target_value, true, true);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::ConsumerGetRequest request_data{
        std::string(target_path)};
    yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
        consumer->Get(request_data);
    ASSERT_TRUE(ret.IsOk());
    auto response = ret.TakeOk().value();
    ASSERT_EQ(1, response.GetDataPoints().size());
    const auto &data_points = response.GetDataPoints();
    ASSERT_EQ(target_path, data_points.at(0).GetPath());
    const auto &val = data_points.at(0).GetData();
    ASSERT_TRUE(yoriito::ValueTypeEquals(true, val));
    bool is_available_sensor = data_points.at(0).IsAvailableSensor();
    ASSERT_TRUE(is_available_sensor);
    bool is_available_actuator = data_points.at(0).IsAvailableActuator();
    ASSERT_TRUE(is_available_actuator);
  }
}
TEST_F(TestConsumerGet, GetActuatorAvailabilityFalseTrue) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";
  {
    bool target_value = false;
    yoriito::databroker::test_stub::ProducerStub stub;
    auto ret = stub.SetData(target_path, target_value, false, true);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::ConsumerGetRequest request_data{
        std::string(target_path)};
    yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
        consumer->Get(request_data);
    ASSERT_TRUE(ret.IsOk());
    auto response = ret.TakeOk().value();
    ASSERT_EQ(1, response.GetDataPoints().size());
    const auto &data_points = response.GetDataPoints();
    ASSERT_EQ(target_path, data_points.at(0).GetPath());
    const auto &val = data_points.at(0).GetData();
    ASSERT_TRUE(yoriito::ValueTypeEquals(false, val));
    bool is_available_sensor = data_points.at(0).IsAvailableSensor();
    ASSERT_FALSE(is_available_sensor);
    bool is_available_actuator = data_points.at(0).IsAvailableActuator();
    ASSERT_TRUE(is_available_actuator);
  }
}
TEST_F(TestConsumerGet, GetActuatorAvailabilityTrueFalse) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";
  {
    bool target_value = true;
    yoriito::databroker::test_stub::ProducerStub stub;
    auto ret = stub.SetData(target_path, target_value, true, false);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::ConsumerGetRequest request_data{
        std::string(target_path)};
    yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
        consumer->Get(request_data);
    ASSERT_TRUE(ret.IsOk());
    auto response = ret.TakeOk().value();
    ASSERT_EQ(1, response.GetDataPoints().size());
    const auto &data_points = response.GetDataPoints();
    ASSERT_EQ(target_path, data_points.at(0).GetPath());
    const auto &val = data_points.at(0).GetData();
    ASSERT_TRUE(yoriito::ValueTypeEquals(true, val));
    bool is_available_sensor = data_points.at(0).IsAvailableSensor();
    ASSERT_TRUE(is_available_sensor);
    bool is_available_actuator = data_points.at(0).IsAvailableActuator();
    ASSERT_FALSE(is_available_actuator);
  }
}

// // timestamp tests
TEST_F(TestConsumerGet, GetActuatorTimestamp) {
  std::string target_path = "Vehicle.Test.SensorAvailabilityBooleanValue";
  int64_t test_timestamp = 10;
  {
    bool target_value = true;
    yoriito::databroker::test_stub::ProducerStub stub;
    auto ret =
        stub.SetData(target_path, target_value, true, true, test_timestamp);
    ASSERT_TRUE(ret.IsOk());
  }
  {
    yoriito::databroker::ConsumerGetRequest request_data{
        std::string(target_path)};
    yoriito::Result<yoriito::databroker::ConsumerGetResponse> ret =
        consumer->Get(request_data);
    ASSERT_TRUE(ret.IsOk());
    auto response = ret.TakeOk().value();
    ASSERT_EQ(1, response.GetDataPoints().size());
    const auto &data_points = response.GetDataPoints();
    ASSERT_EQ(target_path, data_points.at(0).GetPath());
    const auto &val = data_points.at(0).GetData();
    ASSERT_TRUE(yoriito::ValueTypeEquals(true, val));
    bool is_available_sensor = data_points.at(0).IsAvailableSensor();
    ASSERT_TRUE(is_available_sensor);
    bool is_available_actuator = data_points.at(0).IsAvailableActuator();
    ASSERT_TRUE(is_available_actuator);
    std::optional<int64_t> produced_timestamp =
        data_points.at(0).GetProducedTimestamp();
    ASSERT_TRUE(produced_timestamp.has_value());
    ASSERT_EQ(test_timestamp, produced_timestamp.value());
  }
}
