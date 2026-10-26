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

#include "databroker/databroker_producer_factory.h"
#include "producer/inc/viss_producer.h"
#include "yoriito/yoriito_log.h"

class TestProducerDelete : public testing::Test {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer *producer;
};

TEST_F(TestProducerDelete, InvalidPath) {
  std::string target_path = "Vehicle.Length_InValidPath";
  yoriito::databroker::ProducerDeleteRequest data({target_path});
  yoriito::Result<void> ret = producer->Delete(data);
  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
            error.get()->ErrorCode());
}

TEST_F(TestProducerDelete, ValidNodePath) {
  std::string target_path =
      "Vehicle.Test.DeleteTest.SensorStringValue";  // Valid node path
  yoriito::databroker::ProducerDeleteRequest data({target_path});
  yoriito::Result<void> ret = producer->Delete(data);
  ASSERT_TRUE(ret.IsOk());
  // Confirm deletion by trying to get the deleted node
  yoriito::databroker::ProducerGetRequest request_data{
      std::string(target_path)};
  yoriito::Result<yoriito::databroker::ProducerGetResponse> get_ret =
      producer->Get(request_data);
  ASSERT_TRUE(get_ret.IsErr());
  auto get_error = get_ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
            get_error.get()->ErrorCode());
}

TEST_F(TestProducerDelete, ValidBranchPath) {
  std::string target_path = "Vehicle.Test.DeleteTest.Branch";
  yoriito::databroker::ProducerDeleteRequest data({target_path});
  yoriito::Result<void> ret = producer->Delete(data);
  ASSERT_TRUE(ret.IsOk());

  yoriito::databroker::ProducerGetRequest request_data(
      std::string(target_path),
      yoriito::databroker::filter::GetFilter({target_path, "*"}));
  yoriito::Result<yoriito::databroker::ProducerGetResponse> get_ret =
      producer->Get(request_data);
  ASSERT_TRUE(get_ret.IsErr());
  auto get_error = get_ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
            get_error.get()->ErrorCode());
}

TEST_F(TestProducerDelete, ValidMultiplePaths) {
  std::string target_path1 = "Vehicle.Test.DeleteTest.ActuatorBooleanValue";
  std::string target_path2 = "Vehicle.Test.DeleteTest.SensorBooleanValue";

  yoriito::databroker::ProducerDeleteRequest data({target_path1, target_path2});
  yoriito::Result<void> ret = producer->Delete(data);
  ASSERT_TRUE(ret.IsOk());

  {
    yoriito::databroker::ProducerGetRequest request_data{
        std::string(target_path1)};
    yoriito::Result<yoriito::databroker::ProducerGetResponse> get_ret =
        producer->Get(request_data);
    ASSERT_TRUE(get_ret.IsErr());
    auto get_error = get_ret.TakeErr();
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
              get_error.get()->ErrorCode());
  }

  {
    yoriito::databroker::ProducerGetRequest request_data{
        std::string(target_path2)};
    yoriito::Result<yoriito::databroker::ProducerGetResponse> get_ret =
        producer->Get(request_data);
    ASSERT_TRUE(get_ret.IsErr());
    auto get_error = get_ret.TakeErr();
    ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
              get_error.get()->ErrorCode());
  }
}
