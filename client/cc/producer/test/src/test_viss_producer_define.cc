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
#include "yoriito_log.h"
#include "viss_common_log.h"
#include "viss_producer.h"

class TestProducerDefine : public testing::Test {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer* producer;
};

// TODO: define spec is not consistent.
TEST_F(TestProducerDefine, DefineUpdate) {
  std::string target_path = "Vehicle.Length";

  {
    yoriito::databroker::ProducerDefineRequest data({
        yoriito::databroker::ProducerDefineEntry(
            target_path,  // std::string path,

            yoriito::databroker::ProducerDefineEntry::DataType::
                DATA_TYPE_UINT16,  //  DataType data_type,
            yoriito::databroker::ProducerDefineEntry::EntryType::
                ENTRY_TYPE_ATTRIBUTE,  // EntryType entry_type,
            "deprecation",             // std::string deprecation,
            "unit",                    // std::string unit,
            (uint8_t)0,                // ValueType min,
            (uint8_t)0,                // ValueType max,
            "comment",                 // std::string comment,
            {},                        // std::vector<std::string> allowed,
            (uint8_t)0,                // ValueType default_value,
            "description",             // std::string description,
            0,                         // uint32_t arraysize,
            "pattern",                 // std::string pattern,
            {}  // std::map<std::string, std::string> custom_attributes
            )
        // yoriito::databroker::ProducerDefineEntry("key", "value",
        // "description"),
    });
    yoriito::Result<void> ret = producer->Define(data);
    ASSERT_TRUE(ret.IsOk());
  }
  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(
          yoriito::databroker::ProducerInspectRequest{std::string(target_path)});
  ASSERT_TRUE(ret.IsOk());
  auto inspect_response = ret.TakeOk().value();
  ASSERT_EQ(1, inspect_response.GetDataPoints().size());
  auto inspect_data = inspect_response.GetDataPoints()[0];
  ASSERT_EQ(target_path, inspect_data.GetPath());
  ASSERT_EQ(yoriito::databroker::ProducerDefineEntry::DataType::
                DATA_TYPE_UINT16, inspect_data.GetDataType());  //  DataType data_type,
}

// TODO: insert metadata , if specific path is not existed.
TEST_F(TestProducerDefine, DefineInsert) {
  std::string target_path = "Vehicle.not_found_path";

  {
    yoriito::databroker::ProducerDefineRequest data({
        yoriito::databroker::ProducerDefineEntry(
            target_path,  // std::string path,

            yoriito::databroker::ProducerDefineEntry::DataType::
                DATA_TYPE_UINT8,  //  DataType data_type,
            yoriito::databroker::ProducerDefineEntry::EntryType::
                ENTRY_TYPE_ATTRIBUTE,  // EntryType entry_type,
            "deprecation",             // std::string deprecation,
            "unit",                    // std::string unit,
            (uint8_t)0,                // ValueType min,
            (uint8_t)0,                // ValueType max,
            "comment",                 // std::string comment,
            {},                        // std::vector<std::string> allowed,
            (uint8_t)0,                // ValueType default_value,
            "description",             // std::string description,
            0,                         // uint32_t arraysize,
            "pattern",                 // std::string pattern,
            {}  // std::map<std::string, std::string> custom_attributes
            )
        // yoriito::databroker::ProducerDefineEntry("key", "value",
        // "description"),
    });
    yoriito::Result<void> ret = producer->Define(data);
    ASSERT_TRUE(ret.IsOk());
  }

  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(
          yoriito::databroker::ProducerInspectRequest{std::string(target_path)});
  ASSERT_TRUE(ret.IsOk());
}
