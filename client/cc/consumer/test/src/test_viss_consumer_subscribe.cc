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

#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "consumer/test/src/test_copyable_value_type.h"
#include "consumer/test/src/test_producer_stub.h"
#include "databroker/databroker_consumer_factory.h"
#include "yoriito/databroker_type.h"
#include "yoriito/yoriito_log.h"

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

  yoriito::databroker::DataBrokerConsumer *consumer;
};

TEST_F(TestConsumerSubscribe, SubscribeInvalidPath) {
  std::string target_path = "Vehicle.not_found_path";
  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
            error.get()->ErrorCode());
}

TEST_F(TestConsumerSubscribe, Subscribe_Branch_Path) {
  std::string target_path = "Vehicle";
  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
            error.get()->ErrorCode());
}

class TestConsumerSubscribeWithParamSensor
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
    TestConsumerSubscribe, TestConsumerSubscribeWithParamSensor,
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
                        std::vector<std::string>({"AAA", "BBB", "CCC"}))));

TEST_P(TestConsumerSubscribeWithParamSensor, SubscribeValidPathSensor) {
  const auto &params_tuple = GetParam();
  const auto &target_path = std::get<0>(params_tuple);
  const auto &params = std::get<1>(params_tuple);
  yoriito::databroker::test_stub::ProducerStub stub;

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);

  (void)stub.SetData(target_path, yoriito::CloneValueType(params.value));
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
        const auto &data_points = data.GetDataPoints();
        EXPECT_EQ(data_points.at(0).GetPath(), target_path);
        const auto &val = data_points.at(0).GetData();
        EXPECT_TRUE(yoriito::ValueTypeEquals(val, params.value));
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
    TestConsumerSubscribe, TestConsumerSubscribeWithParamActuator,
    ::testing::Values(
        // boolean
        std::make_tuple("Vehicle.Test.ActuatorBooleanValue", true),
        std::make_tuple("Vehicle.Test.ActuatorBooleanValue", false),

        // string
        std::make_tuple("Vehicle.Test.ActuatorStringValue", std::string("")),
        std::make_tuple("Vehicle.Test.ActuatorStringValue",
                        std::string("ABCDEFGHIJK")),

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

        // float
        std::make_tuple("Vehicle.Test.ActuatorFloatValue",
                        static_cast<float>(std::numeric_limits<float>::min())),
        std::make_tuple("Vehicle.Test.ActuatorFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max() /
                                           2)),
        std::make_tuple("Vehicle.Test.ActuatorFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max())),

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

        // bool[]
        std::make_tuple("Vehicle.Test.ActuatorBoolArrayValue",
                        std::vector<bool>({})),
        std::make_tuple("Vehicle.Test.ActuatorBoolArrayValue",
                        std::vector<bool>({true, false, true})),

        // string[]
        std::make_tuple("Vehicle.Test.ActuatorStringArrayValue",
                        std::vector<std::string>({})),
        std::make_tuple("Vehicle.Test.ActuatorStringArrayValue",
                        std::vector<std::string>({"AAA", "BBB", "CCC"})),

        // int8_t[]
        std::make_tuple("Vehicle.Test.ActuatorInt8ArrayValue",
                        std::vector<int8_t>({})),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt8ArrayValue",
            std::vector<int8_t>({
                std::numeric_limits<int8_t>::min(),
                static_cast<int8_t>(std::numeric_limits<int8_t>::max() / 2),
                std::numeric_limits<int8_t>::max(),
            })),

        // int16_t[]
        std::make_tuple("Vehicle.Test.ActuatorInt16ArrayValue",
                        std::vector<int16_t>({})),
        std::make_tuple(
            "Vehicle.Test.ActuatorInt16ArrayValue",
            std::vector<int16_t>({
                std::numeric_limits<int16_t>::min(),
                static_cast<int16_t>(std::numeric_limits<int16_t>::max() / 2),
                std::numeric_limits<int16_t>::max(),
            })),

        // int32_t[]
        std::make_tuple("Vehicle.Test.ActuatorInt32ArrayValue",
                        std::vector<int32_t>({})),
        std::make_tuple("Vehicle.Test.ActuatorInt32ArrayValue",
                        std::vector<int32_t>({
                            std::numeric_limits<int32_t>::min(),
                            std::numeric_limits<int32_t>::max() / 2,
                            std::numeric_limits<int32_t>::max(),
                        })),

        // int64_t[]
        std::make_tuple("Vehicle.Test.ActuatorInt64ArrayValue",
                        std::vector<int64_t>({})),
        std::make_tuple("Vehicle.Test.ActuatorInt64ArrayValue",
                        std::vector<int64_t>({
                            std::numeric_limits<int64_t>::min(),
                            std::numeric_limits<int64_t>::max() / 2,
                            std::numeric_limits<int64_t>::max(),
                        })),

        // uint8_t[]
        std::make_tuple("Vehicle.Test.ActuatorUint8ArrayValue",
                        std::vector<uint8_t>({})),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint8ArrayValue",
            std::vector<uint8_t>({
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::min()),
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2),
                static_cast<uint8_t>(std::numeric_limits<uint8_t>::max()),
            })),

        // uint16_t[]
        std::make_tuple("Vehicle.Test.ActuatorUint16ArrayValue",
                        std::vector<uint16_t>({})),
        std::make_tuple(
            "Vehicle.Test.ActuatorUint16ArrayValue",
            std::vector<uint16_t>({
                std::numeric_limits<uint16_t>::min(),
                static_cast<uint16_t>(std::numeric_limits<uint16_t>::max() / 2),
                std::numeric_limits<uint16_t>::max(),
            })),

        // uint32_t[]
        std::make_tuple("Vehicle.Test.ActuatorUint32ArrayValue",
                        std::vector<uint32_t>({})),
        std::make_tuple("Vehicle.Test.ActuatorUint32ArrayValue",
                        std::vector<uint32_t>({
                            std::numeric_limits<uint32_t>::min(),
                            std::numeric_limits<uint32_t>::max() / 2,
                            std::numeric_limits<uint32_t>::max(),
                        })),

        // uint64_t[]
        std::make_tuple("Vehicle.Test.ActuatorUint64ArrayValue",
                        std::vector<uint64_t>({})),
        std::make_tuple("Vehicle.Test.ActuatorUint64ArrayValue",
                        std::vector<uint64_t>({
                            std::numeric_limits<uint64_t>::min(),
                            std::numeric_limits<uint64_t>::max() / 2,
                            std::numeric_limits<uint64_t>::max(),
                        })),

        // float[]
        std::make_tuple("Vehicle.Test.ActuatorFloatArrayValue",
                        std::vector<float>({})),
        std::make_tuple("Vehicle.Test.ActuatorFloatArrayValue",
                        std::vector<float>({
                            std::numeric_limits<float>::min(),
                            std::numeric_limits<float>::max() / 2,
                            std::numeric_limits<float>::max(),
                        })),

        // double[]
        std::make_tuple("Vehicle.Test.ActuatorDoubleArrayValue",
                        std::vector<double>({})),
        std::make_tuple("Vehicle.Test.ActuatorDoubleArrayValue",
                        std::vector<double>({
                            std::numeric_limits<double>::min(),
                            std::numeric_limits<double>::max() / 2,
                            std::numeric_limits<double>::max(),
                        }))));

TEST_P(TestConsumerSubscribeWithParamActuator, SubscribeValidPathActuator) {
  auto params_tuple = GetParam();
  auto target_path = std::get<0>(params_tuple);
  auto params = std::get<1>(params_tuple);
  yoriito::databroker::test_stub::ProducerStub stub;

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  (void)stub.SetData(target_path, params.value);

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
        const auto &data_points = data.GetDataPoints();
        EXPECT_EQ(data_points.at(0).GetPath(), target_path);
        const auto &val = data_points.at(0).GetData();
        EXPECT_TRUE(yoriito::ValueTypeEquals(val, params.value));
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
    TestConsumerSubscribe, TestConsumerSubscribeWithParamAttribute,
    ::testing::Values(
        // string
        std::make_tuple("Vehicle.Test.AttributeStringValue", std::string("")),
        std::make_tuple("Vehicle.Test.AttributeStringValue",
                        std::string("ABCDEFGHIJK")),

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

        // float
        std::make_tuple("Vehicle.Test.AttributeFloatValue",
                        static_cast<float>(std::numeric_limits<float>::min())),
        std::make_tuple("Vehicle.Test.AttributeFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max() /
                                           2)),
        std::make_tuple("Vehicle.Test.AttributeFloatValue",
                        static_cast<float>(std::numeric_limits<float>::max())),

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

        // // string[]
        std::make_tuple("Vehicle.Test.AttributeStringArrayValue",
                        std::vector<std::string>({})),
        std::make_tuple("Vehicle.Test.AttributeStringArrayValue",
                        std::vector<std::string>({"AAA", "BBB", "CCC"})),

        // // uint8_t
        std::make_tuple(
            "Vehicle.Test.AttributeUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::min())),
        std::make_tuple(
            "Vehicle.Test.AttributeUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.AttributeUint8Value",
            static_cast<uint8_t>(std::numeric_limits<uint8_t>::max())),

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

        // // uint32_t
        std::make_tuple(
            "Vehicle.Test.AttributeUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::min())),
        std::make_tuple(
            "Vehicle.Test.AttributeUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.AttributeUint32Value",
            static_cast<uint32_t>(std::numeric_limits<uint32_t>::max())),
        // // int16_t
        std::make_tuple(
            "Vehicle.Test.AttributeInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::min())),
        std::make_tuple(
            "Vehicle.Test.AttributeInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max() / 2)),
        std::make_tuple(
            "Vehicle.Test.AttributeInt16Value",
            static_cast<int16_t>(std::numeric_limits<int16_t>::max()))));

TEST_P(TestConsumerSubscribeWithParamAttribute, SubscribeValidPathAttribute) {
  // TODO:
  // this test sets values to attribute node, so will be failed.

  /// auto params_tuple = GetParam();
  /// auto target_path = std::get<0>(params_tuple);
  /// auto params = std::get<1>(params_tuple);
  /// yoriito::databroker::test_stub::ProducerStub stub;

  /// yoriito::databroker::ConsumerSubscribeRequest
  /// subscribe_request(target_path); auto ret =
  /// consumer->Subscribe(subscribe_request); (void)stub.SetData(target_path,
  /// params); ASSERT_TRUE(ret.IsOk());

  /// auto subscription = ret.TakeOk().value();
  /// std::mutex mtx;
  /// std::condition_variable cv;
  /// bool data_received = false;

  /// std::thread myThread([subscription = subscription.get(), target_path,
  /// params,
  ///                       &data_received, &cv]() {
  ///   {
  ///     auto messages = subscription->Messages();
  ///     EXPECT_TRUE(messages.IsOk());
  ///     if (messages.IsOk()) {
  ///       auto data = messages.TakeOk().value();
  ///       EXPECT_EQ(data.GetDataPoints().size(), 1);
  ///       auto data_points = data.GetDataPoints();
  ///       EXPECT_EQ(data_points.at(0).GetPath(), target_path);
  ///       auto val = data_points.at(0).GetData();
  ///       EXPECT_EQ(val, params);
  ///       data_received = true;
  ///       cv.notify_one();
  ///       return;
  ///     }
  ///     cv.notify_one();
  ///     return;
  ///   }
  /// });

  /// std::unique_lock<std::mutex> lock(mtx);
  /// auto timeout_duration = std::chrono::seconds(1);
  /// bool wait_result = cv.wait_for(lock, timeout_duration,
  ///                                [&data_received] { return data_received;
  ///                                });
  /// EXPECT_TRUE(wait_result);
  /// auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  /// EXPECT_TRUE(ret_unsub.IsOk());
  /// if (!wait_result) {
  ///   cv.wait_for(lock, timeout_duration,
  ///               [&data_received] { return data_received; });
  /// }
  /// myThread.join();
}
//
TEST_F(TestConsumerSubscribe, SubscribeSensorPathRetainedValue) {
  std::string target_path = "Vehicle.Test.SensorFloatValue";
  float target_value = 10.0f;

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);

  ASSERT_TRUE(ret.IsOk());
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

TEST_F(TestConsumerSubscribe, SubscribeStructActuatorPath) {
  const std::string target_path = "Vehicle.Test.StructSignalActuator";

  yoriito::StructData target_struct;
  target_struct.SetProperty("Latitude", yoriito::ValueType(35.681236));
  target_struct.SetProperty("Longitude", yoriito::ValueType(139.767125));
  target_struct.SetProperty("Altitude", yoriito::ValueType(12.5));
  yoriito::test::CopyableValueType target_value(
      std::make_unique<yoriito::StructData>(std::move(target_struct)));

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();

  {
    yoriito::databroker::test_stub::ProducerStub stub;
    (void)stub.SetData(target_path,
                       yoriito::CloneValueType(target_value.value));
  }

  std::mutex mtx;
  std::condition_variable cv;
  bool data_received = false;

  std::thread myThread([subscription = subscription.get(), target_path,
                        target_value, &data_received, &cv]() {
    auto messages = subscription->Messages();
    EXPECT_TRUE(messages.IsOk());
    if (messages.IsOk()) {
      auto data = messages.TakeOk().value();
      EXPECT_EQ(data.GetDataPoints().size(), 1);
      const auto &data_points = data.GetDataPoints();
      EXPECT_EQ(data_points.at(0).GetPath(), target_path);
      const auto &val = data_points.at(0).GetData();
      EXPECT_TRUE(yoriito::ValueTypeEquals(val, target_value.value));
      data_received = true;
    }
    cv.notify_one();
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

TEST_F(TestConsumerSubscribe, SubscribeNestedStructActuatorPath) {
  const std::string target_path = "Vehicle.Test.NestedStructSignalActuator";

  yoriito::StructData location_value;
  location_value.SetProperty("Latitude", yoriito::ValueType(35.681236));
  location_value.SetProperty("Longitude", yoriito::ValueType(139.767125));
  location_value.SetProperty("Altitude", yoriito::ValueType(12.5));

  yoriito::StructData nested_value;
  nested_value.SetProperty("property1", yoriito::ValueType(123.45));
  nested_value.SetProperty("property2", std::make_unique<yoriito::StructData>(
                                            std::move(location_value)));
  yoriito::test::CopyableValueType target_value(
      std::make_unique<yoriito::StructData>(std::move(nested_value)));

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();

  {
    yoriito::databroker::test_stub::ProducerStub stub;
    (void)stub.SetData(target_path,
                       yoriito::CloneValueType(target_value.value));
  }

  std::mutex mtx;
  std::condition_variable cv;
  bool data_received = false;

  std::thread myThread([subscription = subscription.get(), target_path,
                        target_value, &data_received, &cv]() {
    auto messages = subscription->Messages();
    EXPECT_TRUE(messages.IsOk());
    if (messages.IsOk()) {
      auto data = messages.TakeOk().value();
      EXPECT_EQ(data.GetDataPoints().size(), 1);
      const auto &data_points = data.GetDataPoints();
      EXPECT_EQ(data_points.at(0).GetPath(), target_path);
      const auto &val = data_points.at(0).GetData();
      EXPECT_TRUE(yoriito::ValueTypeEquals(val, target_value.value));
      data_received = true;
    }
    cv.notify_one();
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

TEST_F(TestConsumerSubscribe, SubscribeStructAllTypeActuatorPath) {
  const std::string target_path = "Vehicle.Test.ActuatorStructAllTypeSignal";

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

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();

  yoriito::databroker::test_stub::ProducerStub stub;
  auto set_ret =
      stub.SetData(target_path, yoriito::CloneValueType(target_value.value));
  ASSERT_TRUE(set_ret.IsOk());

  std::mutex mtx;
  std::condition_variable cv;
  bool data_received = false;
  std::thread myThread([subscription = subscription.get(), target_path,
                        target_value, &data_received, &cv]() {
    auto messages = subscription->Messages();
    EXPECT_TRUE(messages.IsOk());
    if (messages.IsOk()) {
      auto data = messages.TakeOk().value();
      EXPECT_EQ(1, data.GetDataPoints().size());
      const auto &data_points = data.GetDataPoints();
      EXPECT_EQ(target_path, data_points.at(0).GetPath());
      EXPECT_TRUE(yoriito::ValueTypeEquals(target_value.value,
                                           data_points.at(0).GetData()));
      data_received = true;
    }
    cv.notify_one();
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

TEST_F(TestConsumerSubscribe, SubscribeStructArraySignal) {
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

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);

  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();

  {
    yoriito::databroker::test_stub::ProducerStub stub;
    (void)stub.SetData(target_path,
                       yoriito::CloneValueType(target_value.value));
  }

  std::mutex mtx;
  std::condition_variable cv;
  bool data_received = false;

  std::thread myThread([subscription = subscription.get(), target_path,
                        target_value, &data_received, &cv]() {
    auto messages = subscription->Messages();
    EXPECT_TRUE(messages.IsOk());
    if (messages.IsOk()) {
      auto data = messages.TakeOk().value();
      EXPECT_EQ(data.GetDataPoints().size(), 1);
      const auto &data_points = data.GetDataPoints();
      EXPECT_EQ(data_points.at(0).GetPath(), target_path);
      const auto &val = data_points.at(0).GetData();
      EXPECT_TRUE(yoriito::ValueTypeEquals(val, target_value.value));
      data_received = true;
    }
    cv.notify_one();
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
  yoriito::databroker::test_stub::ProducerStub stub;
  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(
      "Vehicle.Test", yoriito::databroker::filter::SubscribeFilter(
                          yoriito::databroker::filter::PathsFilter(
                              {"SensorFloatValue", "SensorDoubleValue"})));

  auto test_data =
      std::vector<std::pair<std::string, yoriito::test::CopyableValueType>>{
          {"Vehicle.Test.SensorFloatValue", 200.0f},
          {"Vehicle.Test.SensorDoubleValue", (double)100.0}};

  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();

  for (const auto &[path, value] : test_data) {
    (void)stub.SetData(path, yoriito::CloneValueType(value.value));
  }
  for (const auto &[path, value] : test_data) {
    {
      auto messages = subscription->Messages();
      ASSERT_TRUE(messages.IsOk());
      auto data = messages.TakeOk().value();
      ASSERT_EQ(1, data.GetDataPoints().size());

      if (std::holds_alternative<float>(value.value)) {
        ASSERT_EQ(value.value, data.GetDataPoints().front().GetData());
        ASSERT_EQ(path, data.GetDataPoints().front().GetPath());
      } else if (std::holds_alternative<double>(value.value)) {
        ASSERT_EQ(value.value, data.GetDataPoints().front().GetData());
        ASSERT_EQ(path, data.GetDataPoints().front().GetPath());
      }
    }
  }
  auto ret_unsub = consumer->UnSubscribe(std::move(subscription));
  ASSERT_TRUE(ret_unsub.IsOk());
}

// SubscribeSensorPathFilterWildcard
TEST_F(TestConsumerSubscribe, SubscribeSensorPathFilterWildcard) {
  std::string target_path = "Vehicle.Test.SensorFloatValue";
  yoriito::databroker::test_stub::ProducerStub stub;

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(
      "Vehicle.Test", yoriito::databroker::filter::SubscribeFilter(
                          yoriito::databroker::filter::PathsFilter({"*"})));

  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();
  {
    float target_value = 200.0f;
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

TEST_F(TestConsumerSubscribe, SubscribeWithFilterInvalidGlob) {
  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(
      "Vehicle.Test",
      yoriito::databroker::filter::SubscribeFilter(
          yoriito::databroker::filter::PathsFilter({"[invalid"})));

  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_BAD_REQUEST),
            error.get()->ErrorCode());
}

TEST_F(TestConsumerSubscribe, SubscribeSensorPathLongTime) {
  std::string target_path = "Vehicle.Test.SensorFloatValue";
  float target_value = 200.0f;
  yoriito::databroker::test_stub::ProducerStub stub;
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
  std::string target_path = "Vehicle.Test.SensorFloatValue";
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

TEST_F(TestConsumerSubscribe, SubscribeSensorTimeFilter) {
  std::string target_path = "Vehicle.Test.SensorFloatValue";
  float target_value = 200.0f;
  int32_t repeat_count = 3;
  yoriito::databroker::test_stub::ProducerStub stub;

  (void)stub.SetData(target_path, target_value);

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
  std::string target_path = "Vehicle.Test.SensorFloatValue";
  yoriito::databroker::test_stub::ProducerStub stub;
  for (uint16_t i = 0; i < 255; i++) {
    float target_value = i;
    yoriito::databroker::ConsumerSubscribeRequest subscribe_request(
        target_path);
    auto ret = consumer->Subscribe(subscribe_request);
    ASSERT_TRUE(ret.IsOk());
    auto subscription = ret.TakeOk().value();

    {
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
  std::string target_path = "Vehicle.Test.SensorFloatValue";
  float target_value = 1;
  yoriito::databroker::test_stub::ProducerStub stub;

  yoriito::databroker::ConsumerSubscribeRequest subscribe_request(target_path);
  auto ret = consumer->Subscribe(subscribe_request);
  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();

  {
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
