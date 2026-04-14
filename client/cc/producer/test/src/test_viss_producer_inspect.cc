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

using ::testing::TestWithParam;

TEST(TestProducerInspect, InspectInvalidPath) {
  yoriito::databroker::DataBrokerProducer* producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  std::string target_path = "Vehicle.not_found_path3";
  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(
          yoriito::databroker::ProducerInspectRequest{std::string(target_path)});
  ASSERT_TRUE(ret.IsErr());
}

TEST(TestProducerInspect, InspectPathFilter) {
  yoriito::databroker::DataBrokerProducer* producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  std::string target_path = "Vehicle.LowVoltageBattery";
  yoriito::databroker::ProducerInspectRequest inspect_request(
      std::string(target_path), yoriito::databroker::filter::InspectFilter(
                                    {"CurrentCurrent", "CurrentVoltage", "NominalVoltage"}));

  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(inspect_request);
  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();

  auto expected_data = std::vector<yoriito::databroker::BaseInspectData>{
      yoriito::databroker::BaseInspectData(     // 0: Normal Pattern
          "Vehicle.LowVoltageBattery.CurrentCurrent",  // std::string
                                               // path
          yoriito::databroker::BaseInspectData::DataType::
              DATA_TYPE_FLOAT,  // DataType entry_type
          yoriito::databroker::BaseInspectData::EntryType::
              ENTRY_TYPE_SENSOR,  // EntryType
                                  // data_type
          "",                     // std::string deprecation
          "A",              // std::string unit
          std::monostate{},  // ValueType min
          std::monostate{},   // ValueType max
          "",                                           // std::string comment
          {},                // std::vector<std::string> allowed
          std::monostate{},  // ValueType default_value
          "Current current flowing in/out of the low voltage battery. Positive = Current flowing in to battery, e.g. during charging or driving. Negative = Current flowing out of battery, e.g. when using the battery to start a combustion engine.",
          0,   // uint32_t arraysize
          "",  // std::string pattern
          {}),
      yoriito::databroker::BaseInspectData(      // 0: Normal Pattern
          "Vehicle.LowVoltageBattery.CurrentVoltage",  // std::string
                                                // path
          yoriito::databroker::BaseInspectData::DataType::
              DATA_TYPE_FLOAT,  // DataType entry_type
          yoriito::databroker::BaseInspectData::EntryType::
              ENTRY_TYPE_SENSOR,  // EntryType
                                  // data_type
          "",                     // std::string deprecation
          "V",              // std::string unit
          std::monostate{},  // ValueType min
          std::monostate{},   // ValueType max
          "",                                            // std::string comment
          {},                // std::vector<std::string> allowed
          std::monostate{},  // ValueType default_value
          "Current Voltage of the low voltage battery.",
          0,   // uint32_t arraysize
          "",  // std::string pattern
          {}),
      yoriito::databroker::BaseInspectData(    // 0: Normal Pattern
          "Vehicle.LowVoltageBattery.NominalVoltage",  // std::string
                                              // path
          yoriito::databroker::BaseInspectData::DataType::
              DATA_TYPE_UINT16,  // DataType entry_type
          yoriito::databroker::BaseInspectData::EntryType::
              ENTRY_TYPE_ATTRIBUTE,                      // EntryType
                                                      // data_type
          "",                                         // std::string deprecation
          "V",                                  // std::string unit
          std::monostate{},  // ValueType min
          std::monostate{},  // ValueType max
          "Nominal voltage typically refers to voltage of fully charged battery when delivering rated capacity.",
          {},                // std::vector<std::string> allowed
          std::monostate{},  // ValueType default_value
          "Nominal Voltage of the battery.",
          0,                           // uint32_t arraysize
          "",                          // std::string pattern
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

class ProducerInspect_WithParamTest
    : public TestWithParam<yoriito::databroker::BaseInspectData> {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer* producer;
};

TEST_P(ProducerInspect_WithParamTest, InspectVariousParameter) {
  auto param = GetParam();
  auto target_path = param.GetPath();

  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(
          yoriito::databroker::ProducerInspectRequest{std::string(target_path)});

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();

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
    TestProducerInspect, ProducerInspect_WithParamTest,
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
        )                                     //
);
