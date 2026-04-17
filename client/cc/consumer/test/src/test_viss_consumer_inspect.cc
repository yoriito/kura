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
#include "yoriito_log.h"
#include "viss_consumer.h"

using ::testing::TestWithParam;

TEST(TestConsumerInspect, InspectInvalidPath) {
  yoriito::databroker::VissConsumer consumer;
  std::string target_path = "Vehicle.not_found_path";
  yoriito::Result<yoriito::databroker::ConsumerInspectResponse> ret =
      consumer.Inspect(
          yoriito::databroker::ConsumerInspectRequest{std::string(target_path)});
  ASSERT_TRUE(ret.IsErr());
}

TEST(TestConsumerInspect, InspectPathFilter) {
  std::string target_path = "Vehicle.CurrentLocation.GNSSReceiver.GNSSLocationType";
  yoriito::databroker::VissConsumer consumer;
  yoriito::databroker::ConsumerInspectRequest inspect_request(
      std::string(target_path), yoriito::databroker::filter::InspectFilter(
                                    {"Latitude", "Longitude"}));

  yoriito::Result<yoriito::databroker::ConsumerInspectResponse> ret =
      consumer.Inspect(inspect_request);
  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();

  auto expected_data = std::vector<yoriito::databroker::BaseInspectData>{
      yoriito::databroker::BaseInspectData(     // 0: Normal Pattern
          "Vehicle.CurrentLocation.GNSSReceiver.GNSSLocationType.Latitude",  // std::string
                                               // path
          yoriito::databroker::BaseInspectData::DataType::
              DATA_TYPE_DOUBLE,  // DataType entry_type
          yoriito::databroker::BaseInspectData::EntryType::
              ENTRY_TYPE_SENSOR,  // EntryType
                                  // data_type
          "",                     // std::string deprecation
          "degrees",              // std::string unit
          yoriito::ValueType(static_cast<double>(-90)),  // ValueType min
          yoriito::ValueType(static_cast<double>(90)),   // ValueType max
          "",                                           // std::string comment
          {},                // std::vector<std::string> allowed
          std::monostate{},  // ValueType default_value
          "Current latitude of vehicle in WGS 84 geodetic coordinates, as "
          "measured at the position of GNSS receiver antenna.",  // std::string
                                                                 // description
          0,   // uint32_t arraysize
          "",  // std::string pattern
          {}),
      yoriito::databroker::BaseInspectData(      // 0: Normal Pattern
          "Vehicle.CurrentLocation.GNSSReceiver.GNSSLocationType.Longitude",  // std::string
                                                // path
          yoriito::databroker::BaseInspectData::DataType::
              DATA_TYPE_DOUBLE,  // DataType entry_type
          yoriito::databroker::BaseInspectData::EntryType::
              ENTRY_TYPE_SENSOR,  // EntryType
                                  // data_type
          "",                     // std::string deprecation
          "degrees",              // std::string unit
          yoriito::ValueType(static_cast<double>(-180)),  // ValueType min
          yoriito::ValueType(static_cast<double>(180)),   // ValueType max
          "",                                            // std::string comment
          {},                // std::vector<std::string> allowed
          std::monostate{},  // ValueType default_value
          "Current longitude of vehicle in WGS 84 geodetic coordinates, as "
          "measured at the position of GNSS receiver antenna.",  // std::string
                                                                 // description
          0,   // uint32_t arraysize
          "",  // std::string pattern
          {}),
  };
  ASSERT_EQ(expected_data.size(), data.GetDataPoints().size());

  for (const auto& dp : data.GetDataPoints()) {
    for (const auto& expected_db : expected_data) {
      if (dp.GetPath() == expected_db.GetPath()) {
        ASSERT_EQ(expected_db.GetPath(), dp.GetPath());
        ASSERT_EQ(expected_db.GetEntryType(), dp.GetEntryType());
        ASSERT_EQ(expected_db.GetDataType(), dp.GetDataType());
        ASSERT_EQ(expected_db.GetDescription(), dp.GetDescription());
        ASSERT_EQ(expected_db.GetComment(), dp.GetComment());
        ASSERT_EQ(expected_db.GetUnit(), dp.GetUnit());
        auto default_value = expected_db.GetDefaultValue();
        ASSERT_EQ(default_value, dp.GetDefaultValue());
        auto allowed_values = expected_db.GetAllowed();
        ASSERT_EQ(allowed_values, dp.GetAllowed());
        auto min_value = expected_db.GetMin();
        ASSERT_EQ(min_value, dp.GetMin());
        auto max_value = expected_db.GetMax();
        ASSERT_EQ(max_value, dp.GetMax());
      }
    }
  }
}

class ConsumerInspect_WithParamTest
    : public TestWithParam<yoriito::databroker::BaseInspectData> {
 protected:
  void SetUp() override {
    consumer = yoriito::databroker::DataBrokerConsumerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerConsumer* consumer;
};

TEST_P(ConsumerInspect_WithParamTest, InspectVariousParameter) {
  auto param = GetParam();
  auto target_path = param.GetPath();

  yoriito::databroker::VissConsumer consumer;
  yoriito::Result<yoriito::databroker::ConsumerInspectResponse> ret =
      consumer.Inspect(
          yoriito::databroker::ConsumerInspectRequest{std::string(target_path)});

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  ASSERT_EQ(param.GetPath(), data.GetDataPoints()[0].GetPath());
  ASSERT_EQ(param.GetEntryType(), data.GetDataPoints()[0].GetEntryType());
  ASSERT_EQ(param.GetDataType(), data.GetDataPoints()[0].GetDataType());
  ASSERT_EQ(param.GetDescription(), data.GetDataPoints()[0].GetDescription());
  ASSERT_EQ(param.GetComment(), data.GetDataPoints()[0].GetComment());
  ASSERT_EQ(param.GetUnit(), data.GetDataPoints()[0].GetUnit());
  auto default_value = data.GetDataPoints()[0].GetDefaultValue();
  ASSERT_EQ(param.GetDefaultValue(), default_value);

  auto allowed_values = data.GetDataPoints()[0].GetAllowed();
  ASSERT_EQ(param.GetAllowed(), allowed_values);
  auto min_value = data.GetDataPoints()[0].GetMin();
  ASSERT_EQ(param.GetMin(), min_value);
  auto max_value = data.GetDataPoints()[0].GetMax();
  ASSERT_EQ(param.GetMax(), max_value);
}

INSTANTIATE_TEST_SUITE_P(
    TestConsumerInspect, ConsumerInspect_WithParamTest,
    ::testing::Values(                        //
        yoriito::databroker::BaseInspectData(  // 0: Normal Pattern
            "Vehicle.Powertrain.CombustionEngine."
            "EngineCode",  // std::string
                           // path
            yoriito::databroker::BaseInspectData::DataType::
                DATA_TYPE_STRING,  // DataType entry_type
            yoriito::databroker::BaseInspectData::EntryType::
                ENTRY_TYPE_ATTRIBUTE,  // EntryType
                                       // data_type
            "",                        // std::string deprecation
            "",                        // std::string unit
            std::monostate{},          // ValueType min
            std::monostate{},          // ValueType max
            "For hybrid vehicles the engine code may refer to the "
            "combination "
            "of combustion and electric engine.",  // std::string
                                                   // comment
            {},                // std::vector<std::string> allowed
            std::monostate{},  // ValueType default_value
            "Engine code designation, as specified by "
            "vehicle manufacturer.",  // std::string
                                      // description
            0,                        // uint32_t arraysize
            "",                       // std::string pattern
            {}),

        yoriito::databroker::BaseInspectData(  // 1: Min/Max Pattern ,
                                              // Unit
            "Vehicle.Powertrain.Transmission."
            "TorqueDistribution",  // std::string
                                   // path
            yoriito::databroker::BaseInspectData::DataType::
                DATA_TYPE_FLOAT,  // DataType entry_type
            yoriito::databroker::BaseInspectData::EntryType::
                ENTRY_TYPE_ACTUATOR,  // EntryType
                                      // data_type
            "",                       // std::string deprecation
            "percent",                // std::string unit
            yoriito::ValueType(static_cast<float>(-100)),  // ValueType min
            yoriito::ValueType(static_cast<float>(100)),   // ValueType max
            "",                                           // std::string
                                                          // comment
            {},                // std::vector<std::string> allowed
            std::monostate{},  // ValueType default_value
            "Torque distribution between front and rear axle in "
            "percent. -100% "
            "= Full torque to front axle, 0% = 50:50 Front/Rear, "
            "100% = Full "
            "torque to rear axle.",  // std::string
                                     // description
            0,                       // uint32_t arraysize
            "",                      // std::string pattern
            {})                      //
        ,
        yoriito::databroker::BaseInspectData(  // 2: Allowed Values Pattern ,
                                              // Default Value
            "Vehicle.Powertrain.CombustionEngine."
            "Configuration",  // std::string
                              // path
            yoriito::databroker::BaseInspectData::DataType::
                DATA_TYPE_STRING,  // DataType entry_type
            yoriito::databroker::BaseInspectData::EntryType::
                ENTRY_TYPE_ATTRIBUTE,  // EntryType
                                       // data_type
            "",                        // std::string deprecation
            "",                        // std::string unit
            std::monostate{},          // ValueType min
            std::monostate{},          // ValueType max
            "",                        // std::string
                                       // comment
            {"UNKNOWN", "STRAIGHT", "V", "BOXER", "W", "ROTARY", "RADIAL",
             "SQUARE", "H", "U", "OPPOSED", "X"},
            // std::vector<std::string> allowed
            yoriito::ValueType{
                std::string("UNKNOWN")},  // ValueType default_value
            "Engine configuration.",      // std::string
                                          // description
            0,                            // uint32_t arraysize
            "",                           // std::string pattern
            {})                           //
        //,
        // NOTE: this node seems to be deleted.
        //yoriito::databroker::BaseInspectData(     // 4: Deprecated Pattern
        //    "Vehicle.Driver.Identifier.Issuer",  // std::string
        //                                         // path
        //    yoriito::databroker::BaseInspectData::DataType::
        //        DATA_TYPE_STRING,  // DataType entry_type
        //    yoriito::databroker::BaseInspectData::EntryType::
        //        ENTRY_TYPE_SENSOR,  // EntryType
        //                            // data_type
        //    "v5.0 - use data from "
        //    "Vehicle.Occupant.*.*.Identifier.",  // std::string
        //                                         // deprecation
        //    "",                                  // std::string unit
        //    std::monostate{},                    // ValueType min
        //    std::monostate{},                    // ValueType max
        //    "",                                  // std::string
        //                                         // comment
        //    {},
        //    // std::vector<std::string> allowed
        //    std::monostate{},  // ValueType default_value
        //    "Unique Issuer for the authentication of the "
        //    "occupant e.g. "
        //    "https://accounts.funcorp.com.",  // std::string
        //                                      // description
        //    0,                                // uint32_t arraysize
        //    "",                               // std::string pattern
        //    {})                               //
        )                                     //
);
