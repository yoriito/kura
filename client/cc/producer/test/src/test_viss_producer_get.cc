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
//

#include <gtest/gtest.h>

#include <string>

#include "databroker/databroker_producer_factory.h"
#include "databroker_type.h"
#include "test_consumer_stub.h"

//
using ::testing::Bool;
using ::testing::Combine;
using ::testing::TestWithParam;
using ::testing::Values;

//
class TestProducerGet : public testing::Test {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer* producer;
};

TEST_F(TestProducerGet, GetInvalidPath) {
  std::string target_path = "Vehicle.not_found_path2";

  yoriito::databroker::ProducerGetRequest request_data{std::string(target_path)};
  yoriito::Result<yoriito::databroker::ProducerGetResponse> ret =
      producer->Get(request_data);

  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND), error.get()->ErrorCode());
}

TEST_F(TestProducerGet, GetNotSetValue) {
  std::string target_path =
      "Vehicle.Cabin.Door.Row2.DriverSide.Window.Position";

  yoriito::databroker::ProducerGetRequest request_data{std::string(target_path)};
  yoriito::Result<yoriito::databroker::ProducerGetResponse> ret =
      producer->Get(request_data);

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(0, data.GetDataPoints().size());
}

class TestProducerGetWithParamSensor
    : public TestWithParam<std::tuple<std::string, yoriito::ValueType>> {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer* producer;
};

INSTANTIATE_TEST_SUITE_P(
    TestProducerGet, TestProducerGetWithParamSensor,
    ::testing::Values(
        // boolean
        std::make_tuple("Vehicle.Powertrain.FuelSystem.IsFuelPortFlapOpen", true),
        std::make_tuple("Vehicle.Powertrain.FuelSystem.IsFuelPortFlapOpen", false),

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

TEST_P(TestProducerGetWithParamSensor, GetValidPathSensor) {
  // TOOD:
  // test parameters are invalid. 
  // nodes declared in params are not actuator node, so kura server failed to set them.

  //auto params_tuple = GetParam();
  //auto target_path = std::get<0>(params_tuple);
  //auto params = std::get<1>(params_tuple);
  //yoriito::databroker::test_stub::ConsumerStub stub;
  //(void)stub.SetData(target_path, params);

  //yoriito::databroker::ProducerGetRequest request_data{std::string(target_path)};
  //yoriito::Result<yoriito::databroker::ProducerGetResponse> ret =
  //    producer->Get(request_data);

  //ASSERT_TRUE(ret.IsOk());
  //auto data = ret.TakeOk().value();
  //ASSERT_EQ(data.GetDataPoints().size(), 1);
  //auto data_points = data.GetDataPoints();
  //ASSERT_EQ(data_points.at(0).GetPath(), target_path);
  //auto val = data_points.at(0).GetData();
  //ASSERT_EQ(val, params);
}

class TestProducerGetWithParamActuator
    : public TestWithParam<std::tuple<std::string, yoriito::ValueType>> {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer* producer;
};
INSTANTIATE_TEST_SUITE_P(
    TestProducerGet, TestProducerGetWithParamActuator,
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

TEST_P(TestProducerGetWithParamActuator, GetValidPathActuator) {
  auto params_tuple = GetParam();
  auto target_path = std::get<0>(params_tuple);
  auto params = std::get<1>(params_tuple);
  yoriito::databroker::test_stub::ConsumerStub stub;
  (void)stub.SetData(target_path, params);

  yoriito::databroker::ProducerGetRequest request_data{std::string(target_path)};
  yoriito::Result<yoriito::databroker::ProducerGetResponse> ret =
      producer->Get(request_data);

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  auto data_points = data.GetDataPoints();
  ASSERT_EQ(target_path, data_points.at(0).GetPath());
  auto val = data_points.at(0).GetData();
  ASSERT_EQ(params, val);
}

class TestProducerGetWithParamAttribute
    : public TestWithParam<std::tuple<std::string, yoriito::ValueType>> {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer* producer;
};

INSTANTIATE_TEST_SUITE_P(
    TestProducerGet, TestProducerGetWithParamAttribute,
    ::testing::Values(
        // string
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.EngineCode",
                        std::string("")),
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.EngineCode",
                        std::string("ABCDEFGHIJK")),

        // uint16_t
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.Displacement",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::min())),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.Displacement",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Powertrain.CombustionEngine.Displacement",
            static_cast<uint16_t>(std::numeric_limits<uint16_t>::max())),

        // float
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.StrokeLength",
                        static_cast<float>(std::numeric_limits<float>::min())),
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.StrokeLength",
                        static_cast<float>(std::numeric_limits<float>::max() /
                                           2)),
        std::make_tuple("Vehicle.Powertrain.CombustionEngine.StrokeLength",
                        static_cast<float>(std::numeric_limits<float>::max())),

        // int8_t
        std::make_tuple(
            "Vehicle.Powertrain.Transmission.GearCount",
            static_cast<int8_t>(std::numeric_limits<int8_t>::min())),
        std::make_tuple("Vehicle.Powertrain.Transmission.GearCount",
                        static_cast<int8_t>(std::numeric_limits<int8_t>::max() /
                                            2)),
        std::make_tuple(
            "Vehicle.Powertrain.Transmission.GearCount",
            static_cast<int8_t>(std::numeric_limits<int8_t>::max())),

        // // string[]
        std::make_tuple("Vehicle.Powertrain.TractionBattery.Charging."
                        "ChargingPort.FrontLeft.SupportedInletTypes",
                        std::vector<std::string>({})),
        std::make_tuple("Vehicle.Powertrain.TractionBattery.Charging."
                        "ChargingPort.FrontLeft.SupportedInletTypes",
                        std::vector<std::string>({"AAA", "BBB", "CCC"})),

        // // uint8_t
        std::make_tuple(
            "Vehicle.Cabin.DoorCount",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::min())),
        std::make_tuple(
            "Vehicle.Cabin.DoorCount",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Cabin.DoorCount",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max())),

        // uint8_t[]
        std::make_tuple("Vehicle.Cabin.SeatPosCount", std::vector<uint8_t>({})),
        std::make_tuple(
            "Vehicle.Cabin.SeatPosCount",
            std::vector<uint8_t>({
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::min()),
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2),
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::max()),
            })),

        // // uint32_t
        std::make_tuple(
            "Vehicle.VersionVSS.Major",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::min())),
        std::make_tuple(
            "Vehicle.VersionVSS.Major",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.VersionVSS.Major",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max())),
        // // int16_t
        std::make_tuple(
            "Vehicle.RoofLoad",
            static_cast<int16_t>(std::numeric_limits<int16_t>::min())),
        std::make_tuple(
            "Vehicle.RoofLoad",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.RoofLoad",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max()))));

TEST_P(TestProducerGetWithParamAttribute, GetValidPathAttribute) {
  // TOOD:
  // test parameters are invalid. 
  // nodes declared in params are not actuator node, so kura server failed to set them.

  // auto params_tuple = GetParam();
  // auto target_path = std::get<0>(params_tuple);
  // auto params = std::get<1>(params_tuple);
  // yoriito::databroker::test_stub::ConsumerStub stub;
  // (void)stub.SetData(target_path, params);

  // yoriito::databroker::ProducerGetRequest request_data{std::string(target_path)};
  // yoriito::Result<yoriito::databroker::ProducerGetResponse> ret =
  //     producer->Get(request_data);

  // ASSERT_TRUE(ret.IsOk());
  // auto data = ret.TakeOk().value();
  // ASSERT_EQ(data.GetDataPoints().size(), 1);
  // auto data_points = data.GetDataPoints();
  // ASSERT_EQ(data_points.at(0).GetPath(), target_path);
  // auto val = data_points.at(0).GetData();
  // ASSERT_EQ(val, params);
}
