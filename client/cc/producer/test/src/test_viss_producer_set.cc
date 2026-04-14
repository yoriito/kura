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

#include "databroker/databroker_producer_factory.h"
#include "databroker_type.h"
#include "test_consumer_stub.h"
#include "viss_common_log.h"

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

  yoriito::databroker::DataBrokerProducer* producer;
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
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND), error.get()->ErrorCode());
}
class TestProducerSetWithParamSensor
    : public TestWithParam<std::tuple<std::string, yoriito::ValueType>> {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer* producer;
};

INSTANTIATE_TEST_SUITE_P(
    TestProducerSet, TestProducerSetWithParamSensor,
    ::testing::Values(
        // boolean
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.IsRunning", true),
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.IsRunning", false),

        // uint16_t
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.Speed",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::min())),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.Speed",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.Speed",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max())),

        // float
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.EngineHours",
                        static_cast<float>(std::numeric_limits<float>::min())),
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.EngineHours",
                        static_cast<float>(std::numeric_limits<float>::max() /
                                           2)),
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.EngineHours",
                        static_cast<float>(std::numeric_limits<float>::max())),

        // string
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.EngineOil.Level",
                        std::string("")),
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.EngineOil.Level",
                        std::string("ABCDEFGHIJK")),

        // int32_t
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.EngineOil.LifeRemaining",
            static_cast<int32_t>(std::numeric_limits<int32_t>::min())),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.EngineOil.LifeRemaining",
            static_cast<int32_t>(std::numeric_limits<int32_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.EngineOil.LifeRemaining",
            static_cast<int32_t>(std::numeric_limits<int32_t>::max())),

        // uint8_t
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.TPS",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::min())),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.TPS",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.TPS",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max())),

        // int16_t
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.Torque",
            static_cast<int16_t>(std::numeric_limits<int16_t>::min())),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.Torque",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.Torque",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max())),

        // uint32_t
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.DieselExhaustFluid.Range",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::min())),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.DieselExhaustFluid.Range",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.DieselExhaustFluid.Range",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max())),

        // int8_t
        std::make_tuple(
            "Vehicle.Powertrain.Transmission.CurrentGear",
            static_cast<int8_t>(std::numeric_limits<int8_t>::min())),
        std::make_tuple("Vehicle.Powertrain.Transmission.CurrentGear",
                        static_cast<int8_t>(std::numeric_limits<int8_t>::max() /
                                            2)),
        std::make_tuple(
            "Vehicle.Powertrain.Transmission.CurrentGear",
            static_cast<int8_t>(std::numeric_limits<int8_t>::max())),

        // double
        std::make_tuple(
            "Vehicle.Powertrain.TractionBattery.Charging.Location.Latitude",
            static_cast<double>(std::numeric_limits<double>::min())),
        std::make_tuple(
            "Vehicle.Powertrain.TractionBattery.Charging.Location.Latitude",
            static_cast<double>(std::numeric_limits<double>::max() / 2)),
        std::make_tuple(
            "Vehicle.Powertrain.TractionBattery.Charging.Location.Latitude",
            static_cast<double>(std::numeric_limits<double>::max())),
        // string[]
        std::make_tuple("Vehicle.Powertrain.TractionBattery.Number1.ErrorCodes",
                        std::vector<std::string>({})),
        std::make_tuple("Vehicle.Powertrain.TractionBattery.Number1.ErrorCodes",
                        std::vector<std::string>({"AAA", "BBB", "CCC"}))));

TEST_P(TestProducerSetWithParamSensor, SetSensorPath) {
  auto params_tuple = GetParam();
  auto target_path = std::get<0>(params_tuple);
  auto params = std::get<1>(params_tuple);

  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, params, true,
                                                   true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  yoriito::databroker::test_stub::ConsumerStub stub;
  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  ASSERT_EQ(params, data);

  // producer
}

class TestProducerSetWithParamActuator
    : public TestWithParam<std::tuple<std::string, yoriito::ValueType>> {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer* producer;
};
INSTANTIATE_TEST_SUITE_P(
    TestProducerSet, TestProducerSetWithParamActuator,
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

TEST_P(TestProducerSetWithParamActuator, SetActuatorPath) {
  auto params_tuple = GetParam();
  auto target_path = std::get<0>(params_tuple);
  auto params = std::get<1>(params_tuple);

  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, params, true,
                                                   true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  yoriito::databroker::test_stub::ConsumerStub stub;
  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  ASSERT_EQ(params, data);

  // producer
}
class TestProducerSetWithParamAttribute
    : public TestWithParam<std::tuple<std::string, yoriito::ValueType>> {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer* producer;
};
INSTANTIATE_TEST_SUITE_P(
    TestProducerSet, TestProducerSetWithParamAttribute,
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
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max()))

            ));

TEST_P(TestProducerSetWithParamAttribute, SetAttributePath) {
  auto params_tuple = GetParam();
  auto target_path = std::get<0>(params_tuple);
  auto params = std::get<1>(params_tuple);

  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, params, true,
                                                   true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  yoriito::databroker::test_stub::ConsumerStub stub;
  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();

  ASSERT_EQ(params, data);

  // producer
}
TEST_F(TestProducerSet, SetActuatorPath) {
  std::string target_path = "Vehicle.PowerOptimizeLevel";
  uint8_t target_value = 10;
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, target_value,
                                                   true, true, 0)});

  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  yoriito::databroker::test_stub::ConsumerStub stub;
  auto val = stub.GetData(target_path);
  ASSERT_TRUE(val.IsOk());
  auto data = val.TakeOk().value();
  auto* p = std::get_if<uint8_t>(&data);
  ASSERT_NE(p, nullptr);
  ASSERT_EQ(target_value, *p);
}

TEST_F(TestProducerSet, SetMultiplePathsExistedExisted) {
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(
              "Vehicle.ADAS.ABS.IsEnabled", true, true, true, 0),
          yoriito::databroker::ProducerSetDataPoint(
              "Vehicle.ADAS.ABS.IsEngaged", true, true, true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsOk());
  }

  {
    bool target_value = true;
    yoriito::databroker::test_stub::ConsumerStub stub;
    auto val = stub.GetData("Vehicle.ADAS.ABS.IsEnabled");
    ASSERT_TRUE(val.IsOk());
    auto data = val.TakeOk().value();

    auto* p = std::get_if<bool>(&data);
    ASSERT_NE(nullptr, p);
    ASSERT_EQ(target_value, *p);
  }

  {
    bool target_value = true;
    yoriito::databroker::test_stub::ConsumerStub stub;
    auto val = stub.GetData("Vehicle.ADAS.ABS.IsEngaged");
    ASSERT_TRUE(val.IsOk());
    auto data = val.TakeOk().value();

    auto* p = std::get_if<bool>(&data);
    ASSERT_NE(nullptr, p);
    ASSERT_EQ(target_value, *p);
  }
}
TEST_F(TestProducerSet, SetMultiplePathsExistedNotExisted) {
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint("Vehicle.PowerOptimizeLevel",
                                                   (uint8_t)255, true, true, 0),
          yoriito::databroker::ProducerSetDataPoint(
              "Vehicle.IsAutoPowerOptimize_", true, true, true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND), error.get()->ErrorCode());
  }
}

TEST_F(TestProducerSet, SetSensorPathUnmatchType) {
  std::string target_path = "Vehicle.Speed";
  yoriito::databroker::ProducerSetRequest actuateList(
      std::vector<yoriito::databroker::ProducerSetDataPoint>{
          yoriito::databroker::ProducerSetDataPoint(target_path, (int8_t)10,
                                                   true, true, 0)});
  {
    yoriito::Result<void> ret = producer->Set(actuateList);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_BAD_REQUEST), error.get()->ErrorCode());
  }
}

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
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND), error.get()->ErrorCode());
  }
}
