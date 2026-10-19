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

#include <string>
#include <thread>
#include <vector>

#include "databroker/databroker_producer_factory.h"
#include "producer/test/src/test_consumer_stub.h"
#include "yoriito/databroker_type.h"
#include "yoriito/error.h"
#include "yoriito/yoriito_log.h"

//
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
class TestProducerUnSubscribe : public testing::Test {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer *producer;
};

TEST_F(TestProducerUnSubscribe, UnSubscribeIllegalCall) {
  auto ret = producer->UnSubscribe(nullptr);
  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  auto error_code = error.get()->ErrorCode();
  auto error_message = error.get()->Message();
  ASSERT_EQ(static_cast<uint32_t>(yoriito::ErrorCode::ERROR_INVAL), error_code);
}

TEST_F(TestProducerUnSubscribe, UnSubscribeNormalPattern) {
  std::string target_path = "Vehicle.Test.ActuatorFloatValue";
  float target_value = 200.0f;
  {
    yoriito::databroker::test_stub::ConsumerStub stub;
    (void)stub.SetData(target_path, target_value);
  }

  yoriito::databroker::ProducerSubscribeRequest subscribe_request(
      target_path, yoriito::databroker::filter::SubscribeFilter(
                       yoriito::databroker::filter::TimeBasedFilter(20000)));
  auto ret = producer->Subscribe(subscribe_request);

  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();
  {
    auto ret = producer->UnSubscribe(std::move(subscription));
    ASSERT_TRUE(ret.IsOk());
  }
}
TEST_F(TestProducerUnSubscribe, UnSubscribeDoubleUnSubscribe) {
  std::string target_path = "Vehicle.Test.ActuatorFloatValue";
  float target_value = 200.0f;
  {
    yoriito::databroker::test_stub::ConsumerStub stub;
    (void)stub.SetData(target_path, target_value);
  }

  yoriito::databroker::ProducerSubscribeRequest subscribe_request(
      target_path, yoriito::databroker::filter::SubscribeFilter(
                       yoriito::databroker::filter::TimeBasedFilter(20000)));
  auto ret = producer->Subscribe(subscribe_request);

  ASSERT_TRUE(ret.IsOk());
  auto subscription = ret.TakeOk().value();
  {
    auto ret = producer->UnSubscribe(std::move(subscription));
    ASSERT_TRUE(ret.IsOk());
  }
  {
    auto ret = producer->UnSubscribe(nullptr);
    ASSERT_TRUE(ret.IsErr());
    auto error = ret.TakeErr();
    auto error_code = error.get()->ErrorCode();
    auto error_message = error.get()->Message();
    ASSERT_EQ(static_cast<uint32_t>(yoriito::ErrorCode::ERROR_INVAL),
              error_code);
  }
}
