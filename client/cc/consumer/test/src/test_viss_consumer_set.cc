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

#include "consumer/test/src/test_copyable_value_type.h"
#include "consumer/test/src/test_producer_stub.h"
#include "databroker/databroker_consumer_factory.h"
#include "yoriito/databroker_type.h"
#include "yoriito/yoriito_log.h"

//
using ::testing::Bool;
using ::testing::Combine;
using ::testing::TestWithParam;
using ::testing::Values;

//
class TestConsumerSet : public testing::Test {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerConsumer *consumer;
};

TEST_F(TestConsumerSet, SetInvalidPath) {
  std::string target_path = "Vehicle.not_found_path";

  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(target_path, (uint8_t)10)});

  yoriito::Result<void> ret = consumer->Set(actuateList);
  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
            error.get()->ErrorCode());
}

TEST_F(TestConsumerSet, SetSensorPath) {
  std::string target_path = "Vehicle.Test.SensorFloatValue";
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(target_path, (float)10)});

  yoriito::Result<void> ret = consumer->Set(actuateList);

  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_BAD_REQUEST),
            error.get()->ErrorCode());
}

//
class TestConsumerSetWithParamActuator
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
    TestConsumerSet, TestConsumerSetWithParamActuator,
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

TEST_P(TestConsumerSetWithParamActuator, SetActuatorPath) {
  const auto &params_tuple = GetParam();
  const auto &target_path = std::get<0>(params_tuple);
  const auto &params = std::get<1>(params_tuple);

  yoriito::databroker::test_stub::ProducerStub stub;
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(
              target_path, yoriito::CloneValueType(params.value))});
  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  ASSERT_TRUE(yoriito::ValueTypeEquals(params.value, data));

  // producer
}

// set multiple path

TEST_F(TestConsumerSet, SetMultiplePathsExistedExisted) {
  yoriito::databroker::test_stub::ProducerStub stub;
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(
              "Vehicle.Test.ActuatorUint8Value", (uint8_t)255),
          yoriito::databroker::ConsumerSetDataPoint(
              "Vehicle.Test.ActuatorBooleanValue", true)});
  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  {
    uint8_t target_value = 255;
    auto val = stub.GetData("Vehicle.Test.ActuatorUint8Value");
    ASSERT_TRUE(val.IsOk());
    auto data = val.TakeOk().value();

    auto *p = std::get_if<uint8_t>(&data);
    ASSERT_NE(nullptr, p);
    ASSERT_EQ(target_value, *p);
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
}
TEST_F(TestConsumerSet, SetMultiplePathsExistedNotExisted) {
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(
              "Vehicle.Test.ActuatorUint8Value", (uint8_t)255),
          yoriito::databroker::ConsumerSetDataPoint(
              "Vehicle.IsAutoPowerOptimize_", true)});
  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
              error.get()->ErrorCode());
  }
}
TEST_F(TestConsumerSet, SetPathUnmatchType) {
  std::string target_path = "Vehicle.Test.ActuatorUint8Value";
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(target_path, (int8_t)10)});

  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_BAD_REQUEST),
              error.get()->ErrorCode());
  }
}

// SetBranchPath
TEST_F(TestConsumerSet, SetBranchPath) {
  std::string target_path = "Vehicle";
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(target_path, (uint8_t)10)});

  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
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

// struct signal test
TEST_F(TestConsumerSet, SetStructAllTypeActuatorPath) {
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

  yoriito::databroker::ConsumerSetRequest request(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(
              target_path, yoriito::CloneValueType(target_value.value))});
  auto set_ret = consumer->Set(request);
  ASSERT_TRUE(set_ret.IsOk());

  auto get_ret = stub.GetData(target_path);
  ASSERT_TRUE(get_ret.IsOk());
  ASSERT_TRUE(
      yoriito::ValueTypeEquals(target_value.value, get_ret.TakeOk().value()));
}

//

TEST_F(TestConsumerSet, SetStructActuatorPath) {
  std::string target_path = "Vehicle.Test.StructSignalActuator";

  yoriito::StructData target_value;
  target_value.SetProperty("Latitude", yoriito::ValueType(35.681236));
  target_value.SetProperty("Longitude", yoriito::ValueType(139.767125));
  target_value.SetProperty("Altitude", yoriito::ValueType(12.5));

  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(
              target_path,
              std::make_unique<yoriito::StructData>(std::move(target_value)))});

  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  yoriito::databroker::test_stub::ProducerStub stub;
  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  auto *p = std::get_if<std::unique_ptr<yoriito::StructData>>(&data);
  ASSERT_NE(nullptr, p);
  auto properties = p->get()->GetProperties();
  ASSERT_EQ(3, properties.size());
  ASSERT_EQ(properties.at("Latitude"), yoriito::ValueType(35.681236));
  ASSERT_EQ(properties.at("Longitude"), yoriito::ValueType(139.767125));
  ASSERT_EQ(properties.at("Altitude"), yoriito::ValueType(12.5));
}

// SetNestedStructActuatorSignal
TEST_F(TestConsumerSet, SetNestedStructActuatorPath) {
  std::string target_path = "Vehicle.Test.NestedStructSignalActuator";
  yoriito::databroker::test_stub::ProducerStub stub;

  yoriito::StructData location_value;
  location_value.SetProperty("Latitude", yoriito::ValueType(35.681236));
  location_value.SetProperty("Longitude", yoriito::ValueType(139.767125));
  location_value.SetProperty("Altitude", yoriito::ValueType(12.5));

  yoriito::StructData nested_value;
  nested_value.SetProperty("property1", yoriito::ValueType(123.45));
  nested_value.SetProperty("property2", std::make_unique<yoriito::StructData>(
                                            std::move(location_value)));

  yoriito::databroker::ConsumerSetRequest request(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(
              target_path,
              std::make_unique<yoriito::StructData>(std::move(nested_value)))});

  auto ret = consumer->Set(request);
  ASSERT_TRUE(ret.IsOk());

  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  auto *p = std::get_if<std::unique_ptr<yoriito::StructData>>(&data);
  ASSERT_NE(nullptr, p);
  auto properties = p->get()->GetProperties();
  ASSERT_EQ(2, properties.size());
  ASSERT_EQ(properties.at("property1"), yoriito::ValueType(123.45));
  auto *nested_struct_ptr = std::get_if<std::unique_ptr<yoriito::StructData>>(
      &properties.at("property2"));
  ASSERT_NE(nullptr, nested_struct_ptr);
  auto nested_properties = nested_struct_ptr->get()->GetProperties();
  ASSERT_EQ(3, nested_properties.size());
  ASSERT_EQ(nested_properties.at("Latitude"), yoriito::ValueType(35.681236));
  ASSERT_EQ(nested_properties.at("Longitude"), yoriito::ValueType(139.767125));
  ASSERT_EQ(nested_properties.at("Altitude"), yoriito::ValueType(12.5));
}

//  SetStructArraySignal

TEST_F(TestConsumerSet, SetStructArraySignal) {
  std::string target_path = "Vehicle.Test.StructArrayActutorSignal";
  yoriito::databroker::test_stub::ProducerStub stub;

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

  yoriito::databroker::ConsumerSetRequest request(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(
              target_path, yoriito::CloneValueType(target_value.value))});

  auto ret = consumer->Set(request);
  ASSERT_TRUE(ret.IsOk());

  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();
  ASSERT_TRUE(yoriito::ValueTypeEquals(data, target_value.value));
}

TEST_F(TestConsumerSet, SetActuatorPathUnavailable) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";

  yoriito::databroker::test_stub::ProducerStub stub;
  {
    yoriito::Result<void> ret =
        stub.SetData(target_path, yoriito::ValueType(false), true, false);
    ASSERT_TRUE(ret.IsOk());
  }

  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(target_path, true)});

  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    ASSERT_EQ(
        static_cast<int32_t>(yoriito::ErrorCode::ERROR_SERVICE_UNAVAILABLE),
        error.get()->ErrorCode());
  }
}

TEST_F(TestConsumerSet, SetActuatorPathAvailable) {
  std::string target_path = "Vehicle.Test.ActuatorAvailabilityBooleanValue";

  yoriito::databroker::test_stub::ProducerStub stub;
  {
    yoriito::Result<void> ret =
        stub.SetData(target_path, yoriito::ValueType(false), true, true);
    ASSERT_TRUE(ret.IsOk());
  }

  bool target_value = true;
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(target_path,
                                                    target_value)});

  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  auto *p = std::get_if<bool>(&data);
  ASSERT_NE(nullptr, p);
  ASSERT_EQ(target_value, *p);
}
