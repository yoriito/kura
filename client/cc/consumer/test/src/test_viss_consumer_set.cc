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

#include <string>
#include <vector>

#include "databroker/databroker_consumer_factory.h"
#include "databroker_type.h"
#include "gtest/gtest.h"
#include "yoriito_log.h"
#include "test_producer_stub.h"
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

  yoriito::databroker::DataBrokerConsumer* consumer;
};

TEST_F(TestConsumerSet, SetInvalidPath) {
  yoriito::databroker::DataBrokerConsumer* consumer =
      yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  std::string target_path = "Vehicle.not_found_path";
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(target_path, (uint8_t)10)});

  yoriito::Result<void> ret = consumer->Set(actuateList);
  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND), error.get()->ErrorCode());
}

TEST_F(TestConsumerSet, SetSensorPath) {
  std::string target_path = "Vehicle.Speed";
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(target_path, (float)10)});

  yoriito::Result<void> ret = consumer->Set(actuateList);

  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_BAD_REQUEST), error.get()->ErrorCode());
}

class TestConsumerSetWithParamActuator
    : public TestWithParam<std::tuple<std::string, yoriito::ValueType>> {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerConsumer* consumer;
};
INSTANTIATE_TEST_SUITE_P(
    TestConsumerSet, TestConsumerSetWithParamActuator,
    ::testing::Values(
        // int8_t
        std::make_tuple(
            "Vehicle.Powertrain.Transmission.SelectedGear",
            static_cast<int8_t>(std::numeric_limits<int8_t>::min())),
        std::make_tuple("Vehicle.Powertrain.Transmission.SelectedGear",
                        static_cast<int8_t>(std::numeric_limits<int8_t>::max() /
                                            2)),
        std::make_tuple(
            "Vehicle.Powertrain.Transmission.SelectedGear",
            static_cast<int8_t>(std::numeric_limits<int8_t>::max())),

        // boolean
        std::make_tuple("Vehicle.Powertrain.Transmission.IsParkLockEngaged",
                        true),
        std::make_tuple("Vehicle.Powertrain.Transmission.IsParkLockEngaged",
                        false),

        // string
        std::make_tuple("Vehicle.Powertrain.Transmission.PerformanceMode",
                        std::string("")),
        std::make_tuple("Vehicle.Powertrain.Transmission.PerformanceMode",
                        std::string("ABCDEFGHIJK")),

        // float
        std::make_tuple("Vehicle.Powertrain.Transmission.ClutchEngagement",
                        static_cast<float>(std::numeric_limits<float>::min())),
        std::make_tuple("Vehicle.Powertrain.Transmission.ClutchEngagement",
                        static_cast<float>(std::numeric_limits<float>::max() /
                                           2)),
        std::make_tuple("Vehicle.Powertrain.Transmission.ClutchEngagement",
                        static_cast<float>(std::numeric_limits<float>::max())),

        // uint8_t
        std::make_tuple(
            "Vehicle.Powertrain.TractionBattery.Charging.ChargeLimit",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::min())),
        std::make_tuple(
            "Vehicle.Powertrain.TractionBattery.Charging.ChargeLimit",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Powertrain.TractionBattery.Charging.ChargeLimit",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max())),

        // uint16_t
        std::make_tuple(
            "Vehicle.Cabin.Seat.Row1.DriverSide.SeatBeltHeight",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::min())),
        std::make_tuple(
            "Vehicle.Cabin.Seat.Row1.DriverSide.SeatBeltHeight",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Cabin.Seat.Row1.DriverSide.SeatBeltHeight",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max()))));

TEST_P(TestConsumerSetWithParamActuator, SetActuatorPath) {
  auto params_tuple = GetParam();
  auto target_path = std::get<0>(params_tuple);
  auto params = std::get<1>(params_tuple);

  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(target_path, params)});
  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  yoriito::databroker::test_stub::ProducerStub stub;
  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  ASSERT_EQ(params, data);

  // producer
}

TEST_F(TestConsumerSet, SetMultiplePathsExistedExisted) {
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint("Vehicle.PowerOptimizeLevel",
                                                   (uint8_t)255),
          yoriito::databroker::ConsumerSetDataPoint(
              "Vehicle.IsAutoPowerOptimize", true)});
  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  {
    uint8_t target_value = 255;
    yoriito::databroker::test_stub::ProducerStub stub;
    auto val = stub.GetData("Vehicle.PowerOptimizeLevel");
    ASSERT_TRUE(val.IsOk());
    auto data = val.TakeOk().value();

    auto* p = std::get_if<uint8_t>(&data);
    ASSERT_NE(nullptr, p);
    ASSERT_EQ(target_value, *p);
  }

  {
    bool target_value = true;
    yoriito::databroker::test_stub::ProducerStub stub;
    auto val = stub.GetData("Vehicle.IsAutoPowerOptimize");
    ASSERT_TRUE(val.IsOk());
    auto data = val.TakeOk().value();

    auto* p = std::get_if<bool>(&data);
    ASSERT_NE(nullptr, p);
    ASSERT_EQ(target_value, *p);
  }
}
TEST_F(TestConsumerSet, SetMultiplePathsExistedNotExisted) {
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint("Vehicle.PowerOptimizeLevel",
                                                   (uint8_t)255),
          yoriito::databroker::ConsumerSetDataPoint(
              "Vehicle.IsAutoPowerOptimize_", true)});
  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND), error.get()->ErrorCode());
  }
}
TEST_F(TestConsumerSet, SetActuatorPathUnmatchType) {
  std::string target_path = "Vehicle.PowerOptimizeLevel";
  yoriito::databroker::ConsumerSetRequest actuateList(
      std::vector<yoriito::databroker::ConsumerSetDataPoint>{
          yoriito::databroker::ConsumerSetDataPoint(target_path, (int8_t)10)});

  {
    yoriito::Result<void> ret = consumer->Set(actuateList);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_BAD_REQUEST), error.get()->ErrorCode());
  }
}

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
    // but to return ERROR_BAD_REQUEST, kura server needs to distinguish just not found and found but it is branch.
    // There doesn't seem to be any problem with integrating these two into ERROR_NOT_FOUND, so I'll do that for now.
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND), error.get()->ErrorCode());
  }
}
