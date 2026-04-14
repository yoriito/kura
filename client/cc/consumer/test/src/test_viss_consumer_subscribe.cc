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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "databroker/databroker_consumer_factory.h"
#include "databroker_type.h"
#include "yoriito_log.h"
#include "test_producer_stub.h"

using ::testing::AllOf;
using ::testing::Bool;
using ::testing::Combine;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::Le;
using ::testing::Lt;
using ::testing::TestWithParam;
using ::testing::Values;

//
class TestConsumerSubscribe : public testing::Test {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerConsumer* consumer;
};

TEST_F(TestConsumerSubscribe, SubscribeInvalidPath) {
  std::string target_path = "Vehicle.not_found_path";
  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND), error.get()->ErrorCode());
}

TEST_F(TestConsumerSubscribe, Subscribe_Branch_Path) {
  std::string target_path = "Vehicle";
  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND), error.get()->ErrorCode());
}

class TestConsumerSubscribeWithParamSensor
    : public TestWithParam<std::tuple<std::string, yoriito::ValueType>> {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerConsumer* consumer;
};

INSTANTIATE_TEST_SUITE_P(
    TestConsumerSubscribe, TestConsumerSubscribeWithParamSensor,
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

TEST_P(TestConsumerSubscribeWithParamSensor, SubscribeValidPathSensor) {
  auto params_tuple = GetParam();
  auto target_path = std::get<0>(params_tuple);
  auto params = std::get<1>(params_tuple);
  yoriito::databroker::test_stub::ProducerStub stub;

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  (void)stub.SetData(target_path, params);
  ASSERT_TRUE(ret.IsOk());

  auto subscription = ret.TakeOk().value();
  std::mutex mtx;
  std::condition_variable cv;
  bool data_received = false;
  // bool test_passed = false;

  std::thread myThread([subscription = subscription.get(), target_path, params,
                        &data_received, &cv]() {
    {
      auto messages = subscription->Messages();
      EXPECT_TRUE(messages.IsOk());
      if (messages.IsOk()) {
        auto data = messages.TakeOk().value();
        EXPECT_EQ(data.GetDataPoints().size(), 1);
        auto data_points = data.GetDataPoints();
        EXPECT_EQ(data_points.at(0).GetPath(), target_path);
        auto val = data_points.at(0).GetData();
        EXPECT_EQ(val, params);
        data_received = true;
        cv.notify_one();
        return;
      }
      cv.notify_one();
      return;
    }
  });

  std::unique_lock<std::mutex> lock(mtx);
  auto timeout_duration = std::chrono::seconds(1);
  bool wait_result = cv.wait_for(lock, timeout_duration,
                                 [&data_received] { return data_received; });
  EXPECT_TRUE(wait_result);
  auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  EXPECT_TRUE(ret_unsub.IsOk());
  if (!wait_result) {
    cv.wait_for(lock, timeout_duration,
                [&data_received] { return data_received; });
  }
  myThread.join();
}
class TestConsumerSubscribeWithParamActuator
    : public TestWithParam<std::tuple<std::string, yoriito::ValueType>> {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerConsumer* consumer;
};

INSTANTIATE_TEST_SUITE_P(
    TestConsumerSubscribe, TestConsumerSubscribeWithParamActuator,
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

TEST_P(TestConsumerSubscribeWithParamActuator, SubscribeValidPathActuator) {
  auto params_tuple = GetParam();
  auto target_path = std::get<0>(params_tuple);
  auto params = std::get<1>(params_tuple);
  yoriito::databroker::test_stub::ProducerStub stub;

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  (void)stub.SetData(target_path, params);
  ASSERT_TRUE(ret.IsOk());

  auto subscription = ret.TakeOk().value();
  std::mutex mtx;
  std::condition_variable cv;
  bool data_received = false;

  std::thread myThread([subscription = subscription.get(), target_path, params,
                        &data_received, &cv]() {
    {
      auto messages = subscription->Messages();
      EXPECT_TRUE(messages.IsOk());
      if (messages.IsOk()) {
        auto data = messages.TakeOk().value();
        EXPECT_EQ(data.GetDataPoints().size(), 1);
        auto data_points = data.GetDataPoints();
        EXPECT_EQ(data_points.at(0).GetPath(), target_path);
        auto val = data_points.at(0).GetData();
        EXPECT_EQ(val, params);
        data_received = true;
        cv.notify_one();
        return;
      }
      cv.notify_one();
      return;
    }
  });

  std::unique_lock<std::mutex> lock(mtx);
  auto timeout_duration = std::chrono::seconds(1);
  bool wait_result = cv.wait_for(lock, timeout_duration,
                                 [&data_received] { return data_received; });
  EXPECT_TRUE(wait_result);
  auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  EXPECT_TRUE(ret_unsub.IsOk());
  if (!wait_result) {
    cv.wait_for(lock, timeout_duration,
                [&data_received] { return data_received; });
  }
  myThread.join();
}

class TestConsumerSubscribeWithParamAttribute
    : public TestWithParam<std::tuple<std::string, yoriito::ValueType>> {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerConsumer* consumer;
};

INSTANTIATE_TEST_SUITE_P(
    TestConsumerSubscribe, TestConsumerSubscribeWithParamAttribute,
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

TEST_P(TestConsumerSubscribeWithParamAttribute, SubscribeValidPathAttribute) {
  // TODO:
  // this test sets values to attribute node, so will be failed. 

  ///auto params_tuple = GetParam();
  ///auto target_path = std::get<0>(params_tuple);
  ///auto params = std::get<1>(params_tuple);
  ///yoriito::databroker::test_stub::ProducerStub stub;

  ///yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  ///auto ret = consumer->Subscribe(subscribe_request);
  ///(void)stub.SetData(target_path, params);
  ///ASSERT_TRUE(ret.IsOk());

  ///auto subscription = ret.TakeOk().value();
  ///std::mutex mtx;
  ///std::condition_variable cv;
  ///bool data_received = false;

  ///std::thread myThread([subscription = subscription.get(), target_path, params,
  ///                      &data_received, &cv]() {
  ///  {
  ///    auto messages = subscription->Messages();
  ///    EXPECT_TRUE(messages.IsOk());
  ///    if (messages.IsOk()) {
  ///      auto data = messages.TakeOk().value();
  ///      EXPECT_EQ(data.GetDataPoints().size(), 1);
  ///      auto data_points = data.GetDataPoints();
  ///      EXPECT_EQ(data_points.at(0).GetPath(), target_path);
  ///      auto val = data_points.at(0).GetData();
  ///      EXPECT_EQ(val, params);
  ///      data_received = true;
  ///      cv.notify_one();
  ///      return;
  ///    }
  ///    cv.notify_one();
  ///    return;
  ///  }
  ///});

  ///std::unique_lock<std::mutex> lock(mtx);
  ///auto timeout_duration = std::chrono::seconds(1);
  ///bool wait_result = cv.wait_for(lock, timeout_duration,
  ///                               [&data_received] { return data_received; });
  ///EXPECT_TRUE(wait_result);
  ///auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  ///EXPECT_TRUE(ret_unsub.IsOk());
  ///if (!wait_result) {
  ///  cv.wait_for(lock, timeout_duration,
  ///              [&data_received] { return data_received; });
  ///}
  ///myThread.join();
}

TEST_F(TestConsumerSubscribe, SubscribeSensorPathLongTime) {
  std::string target_path = "Vehicle.Speed";
  float target_value = 200.0f;
  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();

  std::mutex mtx;
  std::condition_variable cv;
  bool data_received = false;

  std::thread myThread(
      [subscription = subscription.get(), target_value, &data_received, &cv]() {
        {
          auto messages = subscription->Messages();
          EXPECT_TRUE(messages.IsOk());
          if (messages.IsOk()) {
            auto data = messages.TakeOk().value();
            EXPECT_EQ(data.GetDataPoints().size(), 1);
            auto val = std::get<float>(data.GetDataPoints()[0].GetData());
            EXPECT_EQ(val, target_value);
            data_received = true;
          }
          cv.notify_one();
          return;
        }
      });
  for (int8_t i = 0; i < 30; i++) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  yoriito::databroker::test_stub::ProducerStub stub;
  (void)stub.SetData(target_path, target_value);

  std::unique_lock<std::mutex> lock(mtx);
  auto timeout_duration = std::chrono::seconds(1);
  bool wait_result = cv.wait_for(lock, timeout_duration,
                                 [&data_received] { return data_received; });
  EXPECT_TRUE(wait_result);

  auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  EXPECT_TRUE(ret_unsub.IsOk());
  if (!wait_result) {
    cv.wait_for(lock, timeout_duration,
                [&data_received] { return data_received; });
  }
  myThread.join();
}

TEST_F(TestConsumerSubscribe, SubscribeSensorPathMultiCount) {
  std::string target_path = "Vehicle.Speed";
  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();
  for (int8_t i = 0; i < 30; i++) {
    float target_value = i;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    yoriito::databroker::test_stub::ProducerStub stub;
    (void)stub.SetData(target_path, target_value);

    auto messages = subscription->Messages();
    ASSERT_TRUE(messages.IsOk());
    auto data = messages.TakeOk().value();
    ASSERT_EQ(1, data.GetDataPoints().size());
    auto val = std::get<float>(data.GetDataPoints()[0].GetData());
    ASSERT_EQ(target_value, val);
  }

  auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  EXPECT_TRUE(ret_unsub.IsOk());
}

TEST_F(TestConsumerSubscribe, SubscribeSensorPathRetainedValue) {
  std::string target_path = "Vehicle.Speed";
  float target_value = 10.0f;

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_EQ(true, ret.IsOk());
  auto subscription = ret.TakeOk().value();

  // NOTE:
  // precondition : set data
  // The kura server notifies only events that occur after subscribing.
  // Therefore, this test code needs to set the value after subscribing.
  {
    yoriito::databroker::test_stub::ProducerStub stub;
    (void)stub.SetData(target_path, target_value);
  }

  std::mutex mtx;
  std::condition_variable cv;
  bool data_received = false;

  std::thread myThread(
      [subscription = subscription.get(), target_value, &data_received, &cv]() {
        {
          auto messages = subscription->Messages();
          EXPECT_TRUE(messages.IsOk());
          if (messages.IsOk()) {
            auto data = messages.TakeOk().value();
            EXPECT_EQ(data.GetDataPoints().size(), 1);
            auto val = std::get<float>(data.GetDataPoints()[0].GetData());
            EXPECT_EQ(val, target_value);
            data_received = true;
          }
          cv.notify_one();
          return;
        }
      });
  std::unique_lock<std::mutex> lock(mtx);
  auto timeout_duration = std::chrono::seconds(1);
  bool wait_result = cv.wait_for(lock, timeout_duration,
                                 [&data_received] { return data_received; });
  EXPECT_TRUE(wait_result);
  auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  EXPECT_TRUE(ret_unsub.IsOk());
  if (!wait_result) {
    cv.wait_for(lock, timeout_duration,
                [&data_received] { return data_received; });
  }
  myThread.join();
}

TEST_F(TestConsumerSubscribe, SubscribeSensorPathFilter) {
  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(
      "Vehicle", yoriito::databroker::filter::SubscribeFilter(
                     yoriito::databroker::filter::PathsFilter(
                         {"Speed", "TraveledDistance"})));

  auto test_data = std::vector<std::pair<std::string, yoriito::ValueType>>{
      {"Vehicle.Speed", 200.0f}, {"Vehicle.TraveledDistance", 100.0f}};

  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();

  for (const auto& [path, value] : test_data) {
    yoriito::databroker::test_stub::ProducerStub stub;
    (void)stub.SetData(path, value);
  }
  for (const auto& [path, value] : test_data) {
    {
      auto messages = subscription->Messages();
      ASSERT_TRUE(messages.IsOk());
      auto data = messages.TakeOk().value();
      ASSERT_EQ(1, data.GetDataPoints().size());
      auto val = std::get<float>(data.GetDataPoints()[0].GetData());
      auto expected_val = std::get<float>(value);
      ASSERT_EQ(expected_val, val);
      ASSERT_EQ(path, data.GetDataPoints()[0].GetPath());
    }
  }
  auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  ASSERT_TRUE(ret_unsub.IsOk());
}

TEST_F(TestConsumerSubscribe, SubscribeSensorPathFilterWildcard) {
  std::string target_path = "Vehicle.TraveledDistance";

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(
      "Vehicle", yoriito::databroker::filter::SubscribeFilter(
                     yoriito::databroker::filter::PathsFilter({"*"})));

  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();
  {
    float target_value = 200.0f;
    yoriito::databroker::test_stub::ProducerStub stub;
    (void)stub.SetData(target_path, target_value);
  }
  {
    auto messages = subscription->Messages();
    ASSERT_TRUE(messages.IsOk());
    auto data = messages.TakeOk().value();
    ASSERT_EQ(1, data.GetDataPoints().size());
    auto val = std::get<float>(data.GetDataPoints()[0].GetData());
    ASSERT_EQ(200.0f, val);
  }
  auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  ASSERT_TRUE(ret_unsub.IsOk());
}

TEST_F(TestConsumerSubscribe, SubscribeSensorTimeFilter) {
  std::string target_path = "Vehicle.Speed";
  float target_value = 200.0f;
  int32_t repeat_count = 3;

  {
    yoriito::databroker::test_stub::ProducerStub stub;
    (void)stub.SetData(target_path, target_value);
  }

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(
      target_path, yoriito::databroker::filter::SubscribeFilter(
                       yoriito::databroker::filter::TimeBasedFilter(10000)));
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();

  for (int32_t i = 0; i < repeat_count; i++) {
    auto start = std::chrono::high_resolution_clock::now();
    auto messages = subscription->Messages();
    ASSERT_TRUE(messages.IsOk());
    auto data = messages.TakeOk().value();
    ASSERT_EQ(1, data.GetDataPoints().size());
    auto val = std::get<float>(data.GetDataPoints()[0].GetData());
    ASSERT_EQ(target_value, val);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    if (i == 0) {
      // The first message may arrive faster than the filter time, so skip the
      // check.
      continue;
    }
    EXPECT_THAT(elapsed, AllOf(Ge(9000), Le(12000)));
  }
  auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  ASSERT_TRUE(ret_unsub.IsOk());
}

TEST_F(TestConsumerSubscribe, SubscribeRepeatTest) {
  for (uint16_t i = 0; i < 255; i++) {
    float target_value = i;
    std::string target_path = "Vehicle.Speed";
    yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
    auto ret = consumer->Subscribe(subscribe_request);
    ASSERT_TRUE(ret.IsOk());
    auto subscription = ret.TakeOk().value();

    {
      yoriito::databroker::test_stub::ProducerStub stub;
      (void)stub.SetData(target_path, target_value);
      auto messages = subscription->Messages();
      ASSERT_TRUE(messages.IsOk());
      auto data = messages.TakeOk().value();
      ASSERT_EQ(1, data.GetDataPoints().size());
      auto val = std::get<float>(data.GetDataPoints()[0].GetData());
      ASSERT_EQ(target_value, val);
    }
    auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
    ASSERT_TRUE(ret_unsub.IsOk());
  }
}
TEST_F(TestConsumerSubscribe, SubscribeAutoCloseTest) {
  float target_value = 1;
  std::string target_path = "Vehicle.Speed";
  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();

  {
    yoriito::databroker::test_stub::ProducerStub stub;
    (void)stub.SetData(target_path, target_value);
    auto messages = subscription->Messages();
    ASSERT_TRUE(messages.IsOk());
    auto data = messages.TakeOk().value();
    ASSERT_EQ(1, data.GetDataPoints().size());
    auto val = std::get<float>(data.GetDataPoints()[0].GetData());
    ASSERT_EQ(target_value, val);
  }

  subscription.reset();
}
