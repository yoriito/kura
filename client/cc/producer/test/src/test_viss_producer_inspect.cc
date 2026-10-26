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

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "databroker/databroker_producer_factory.h"
#include "yoriito/databroker_type.h"

using ::testing::TestWithParam;

TEST(TestProducerInspect, InspectInvalidPath) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  std::string target_path = "Vehicle.not_found_path2";

  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});
  ASSERT_TRUE(ret.IsErr());
}

TEST(TestProducerInspect, InspectBranchPath) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(
          yoriito::databroker::ProducerInspectRequest{"Vehicle.Test"});

  ASSERT_TRUE(ret.IsErr());
  auto error = ret.TakeErr();
  ASSERT_EQ(static_cast<int32_t>(yoriito::ErrorCode::ERROR_NOT_FOUND),
            error.get()->ErrorCode());
}

TEST(TestProducerInspect, InspectPathFilter) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  std::string target_path = "Vehicle.Test";
  yoriito::databroker::ProducerInspectRequest inspect_request(
      std::string(target_path),
      yoriito::databroker::filter::InspectFilter(
          {"ActuatorDoubleValue", "ActuatorFloatValue"}));

  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(inspect_request);
  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();

  auto double_builder = yoriito::databroker::BaseInspectData::CreateBuilder();
  auto float_builder = yoriito::databroker::BaseInspectData::CreateBuilder();

  auto expected_data = std::vector<yoriito::databroker::BaseInspectData>{
      double_builder.SetPath("Vehicle.Test.ActuatorDoubleValue")
          .SetDataType(
              yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_DOUBLE)
          .SetEntryType(yoriito::databroker::BaseInspectData::EntryType::
                            ENTRY_TYPE_ACTUATOR)
          .SetDeprecation("")
          .SetUnit("ratio")
          .SetMin(static_cast<double>(-1000000.0))
          .SetMax(static_cast<double>(1000000.0))
          .SetComment("metadata test")
          .SetAllowed({})
          .SetDefaultValue(std::monostate{})
          .SetDescription("Double property")
          .SetArraySize(0)
          .SetPattern("")
          .SetCustomAttributes({})
          .Build(),
      float_builder.SetPath("Vehicle.Test.ActuatorFloatValue")
          .SetDataType(
              yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_FLOAT)
          .SetEntryType(yoriito::databroker::BaseInspectData::EntryType::
                            ENTRY_TYPE_ACTUATOR)
          .SetDeprecation("")
          .SetUnit("ratio")
          .SetMin(static_cast<float>(-1000.0))
          .SetMax(static_cast<float>(1000.0))
          .SetComment("metadata test")
          .SetAllowed({})
          .SetDefaultValue(std::monostate{})
          .SetDescription("Float property")
          .SetArraySize(0)
          .SetPattern("")
          .SetCustomAttributes({})
          .Build(),
  };
  ASSERT_EQ(expected_data.size(), data.GetDataPoints().size());

  for (const auto &dp : data.GetDataPoints()) {
    for (const auto &expected_db : expected_data) {
      if (dp.GetPath() == expected_db.GetPath()) {
        ASSERT_EQ(expected_db.GetPath(), dp.GetPath());
        ASSERT_EQ(expected_db.GetEntryType(), dp.GetEntryType());
        ASSERT_EQ(expected_db.GetDataType(), dp.GetDataType());
        ASSERT_EQ(expected_db.GetDescription(), dp.GetDescription());
        ASSERT_EQ(expected_db.GetComment(), dp.GetComment());
        ASSERT_EQ(expected_db.GetUnit(), dp.GetUnit());
        const auto &default_value = expected_db.GetDefaultValue();
        ASSERT_EQ(default_value, dp.GetDefaultValue());
        const auto &allowed_values = expected_db.GetAllowed();
        ASSERT_EQ(allowed_values, dp.GetAllowed());
        const auto &min_value = expected_db.GetMin();
        ASSERT_EQ(min_value, dp.GetMin());
        const auto &max_value = expected_db.GetMax();
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

  yoriito::databroker::DataBrokerProducer *producer;
};

TEST_P(ProducerInspect_WithParamTest, InspectVariousParameter) {
  auto param = GetParam();
  auto target_path = param.GetPath();

  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();

  ASSERT_EQ(param.GetPath(), data.GetDataPoints()[0].GetPath());
  ASSERT_EQ(param.GetEntryType(), data.GetDataPoints()[0].GetEntryType());
  ASSERT_EQ(param.GetDataType(), data.GetDataPoints()[0].GetDataType());
  ASSERT_EQ(param.GetDescription(), data.GetDataPoints()[0].GetDescription());
  ASSERT_EQ(param.GetComment(), data.GetDataPoints()[0].GetComment());
  ASSERT_EQ(param.GetUnit(), data.GetDataPoints()[0].GetUnit());
  const auto &default_value = data.GetDataPoints()[0].GetDefaultValue();
  ASSERT_EQ(param.GetDefaultValue(), default_value);

  const auto &allowed_values = data.GetDataPoints()[0].GetAllowed();
  ASSERT_EQ(param.GetAllowed(), allowed_values);
  const auto &min_value = data.GetDataPoints()[0].GetMin();
  ASSERT_EQ(param.GetMin(), min_value);
  const auto &max_value = data.GetDataPoints()[0].GetMax();
  ASSERT_EQ(param.GetMax(), max_value);
}

namespace {
using BaseInspectData = yoriito::databroker::BaseInspectData;
using DataType = BaseInspectData::DataType;
using EntryType = BaseInspectData::EntryType;

BaseInspectData MakeCase(const std::string &path, DataType data_type,
                         EntryType entry_type, const std::string &description,
                         const std::string &unit, yoriito::ValueType min,
                         yoriito::ValueType max,
                         const std::vector<std::string> &allowed,
                         const std::string &pattern = "") {
  return BaseInspectData(path, data_type, entry_type, "", unit, std::move(min),
                         std::move(max), "metadata test", allowed,
                         std::monostate{}, description, 0, pattern, {});
}

std::vector<BaseInspectData> BuildPrimitiveSignalInspectParams() {
  return {
      MakeCase("Vehicle.Test.SensorBooleanValue", DataType::DATA_TYPE_BOOLEAN,
               EntryType::ENTRY_TYPE_SENSOR, "Boolean property", "",
               std::monostate{}, std::monostate{}, {"true", "false"}),
      MakeCase("Vehicle.Test.ActuatorBooleanValue", DataType::DATA_TYPE_BOOLEAN,
               EntryType::ENTRY_TYPE_ACTUATOR, "Boolean property", "",
               std::monostate{}, std::monostate{}, {"true", "false"}),
      MakeCase("Vehicle.Test.AttributeBooleanValue",
               DataType::DATA_TYPE_BOOLEAN, EntryType::ENTRY_TYPE_ATTRIBUTE,
               "Boolean property", "", std::monostate{}, std::monostate{},
               {"true", "false"}),

      MakeCase("Vehicle.Test.SensorStringValue", DataType::DATA_TYPE_STRING,
               EntryType::ENTRY_TYPE_SENSOR, "String property", "",
               std::monostate{}, std::monostate{}, {"", "sample", "abc"},
               "^[A-Za-z0-9_]*$"),
      MakeCase("Vehicle.Test.ActuatorStringValue", DataType::DATA_TYPE_STRING,
               EntryType::ENTRY_TYPE_ACTUATOR, "String property", "",
               std::monostate{}, std::monostate{}, {"", "sample", "abc"},
               "^[A-Za-z0-9_]*$"),
      MakeCase("Vehicle.Test.AttributeStringValue", DataType::DATA_TYPE_STRING,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "String property", "",
               std::monostate{}, std::monostate{}, {"", "sample", "abc"},
               "^[A-Za-z0-9_]*$"),

      MakeCase("Vehicle.Test.SensorInt8Value", DataType::DATA_TYPE_INT8,
               EntryType::ENTRY_TYPE_SENSOR, "Int8 property", "ratio",
               yoriito::ValueType(static_cast<int8_t>(-128)),
               yoriito::ValueType(static_cast<int8_t>(127)), {}),
      MakeCase("Vehicle.Test.ActuatorInt8Value", DataType::DATA_TYPE_INT8,
               EntryType::ENTRY_TYPE_ACTUATOR, "Int8 property", "ratio",
               yoriito::ValueType(static_cast<int8_t>(-128)),
               yoriito::ValueType(static_cast<int8_t>(127)), {}),
      MakeCase("Vehicle.Test.AttributeInt8Value", DataType::DATA_TYPE_INT8,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Int8 property", "ratio",
               yoriito::ValueType(static_cast<int8_t>(-128)),
               yoriito::ValueType(static_cast<int8_t>(127)), {}),

      MakeCase("Vehicle.Test.SensorInt16Value", DataType::DATA_TYPE_INT16,
               EntryType::ENTRY_TYPE_SENSOR, "Int16 property", "ratio",
               yoriito::ValueType(static_cast<int16_t>(-32768)),
               yoriito::ValueType(static_cast<int16_t>(32767)), {}),
      MakeCase("Vehicle.Test.ActuatorInt16Value", DataType::DATA_TYPE_INT16,
               EntryType::ENTRY_TYPE_ACTUATOR, "Int16 property", "ratio",
               yoriito::ValueType(static_cast<int16_t>(-32768)),
               yoriito::ValueType(static_cast<int16_t>(32767)), {}),
      MakeCase("Vehicle.Test.AttributeInt16Value", DataType::DATA_TYPE_INT16,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Int16 property", "ratio",
               yoriito::ValueType(static_cast<int16_t>(-32768)),
               yoriito::ValueType(static_cast<int16_t>(32767)), {}),

      MakeCase("Vehicle.Test.SensorInt32Value", DataType::DATA_TYPE_INT32,
               EntryType::ENTRY_TYPE_SENSOR, "Int32 property", "ratio",
               yoriito::ValueType(static_cast<int32_t>(-2147483647 - 1)),
               yoriito::ValueType(static_cast<int32_t>(2147483647)), {}),
      MakeCase("Vehicle.Test.ActuatorInt32Value", DataType::DATA_TYPE_INT32,
               EntryType::ENTRY_TYPE_ACTUATOR, "Int32 property", "ratio",
               yoriito::ValueType(static_cast<int32_t>(-2147483647 - 1)),
               yoriito::ValueType(static_cast<int32_t>(2147483647)), {}),
      MakeCase("Vehicle.Test.AttributeInt32Value", DataType::DATA_TYPE_INT32,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Int32 property", "ratio",
               yoriito::ValueType(static_cast<int32_t>(-2147483647 - 1)),
               yoriito::ValueType(static_cast<int32_t>(2147483647)), {}),

      MakeCase(
          "Vehicle.Test.SensorInt64Value", DataType::DATA_TYPE_INT64,
          EntryType::ENTRY_TYPE_SENSOR, "Int64 property", "ratio",
          yoriito::ValueType(static_cast<int64_t>(-9223372036854775807 - 1)),
          yoriito::ValueType(static_cast<int64_t>(9223372036854775807)), {}),
      MakeCase(
          "Vehicle.Test.ActuatorInt64Value", DataType::DATA_TYPE_INT64,
          EntryType::ENTRY_TYPE_ACTUATOR, "Int64 property", "ratio",
          yoriito::ValueType(static_cast<int64_t>(-9223372036854775807 - 1)),
          yoriito::ValueType(static_cast<int64_t>(9223372036854775807)), {}),
      MakeCase(
          "Vehicle.Test.AttributeInt64Value", DataType::DATA_TYPE_INT64,
          EntryType::ENTRY_TYPE_ATTRIBUTE, "Int64 property", "ratio",
          yoriito::ValueType(static_cast<int64_t>(-9223372036854775807 - 1)),
          yoriito::ValueType(static_cast<int64_t>(9223372036854775807)), {}),

      MakeCase("Vehicle.Test.SensorUint8Value", DataType::DATA_TYPE_UINT8,
               EntryType::ENTRY_TYPE_SENSOR, "Uint8 property", "ratio",
               yoriito::ValueType(static_cast<uint8_t>(0)),
               yoriito::ValueType(static_cast<uint8_t>(255)), {}),
      MakeCase("Vehicle.Test.ActuatorUint8Value", DataType::DATA_TYPE_UINT8,
               EntryType::ENTRY_TYPE_ACTUATOR, "Uint8 property", "ratio",
               yoriito::ValueType(static_cast<uint8_t>(0)),
               yoriito::ValueType(static_cast<uint8_t>(255)), {}),
      MakeCase("Vehicle.Test.AttributeUint8Value", DataType::DATA_TYPE_UINT8,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Uint8 property", "ratio",
               yoriito::ValueType(static_cast<uint8_t>(0)),
               yoriito::ValueType(static_cast<uint8_t>(255)), {}),

      MakeCase("Vehicle.Test.SensorUint16Value", DataType::DATA_TYPE_UINT16,
               EntryType::ENTRY_TYPE_SENSOR, "Uint16 property", "ratio",
               yoriito::ValueType(static_cast<uint16_t>(0)),
               yoriito::ValueType(static_cast<uint16_t>(65535)), {}),
      MakeCase("Vehicle.Test.ActuatorUint16Value", DataType::DATA_TYPE_UINT16,
               EntryType::ENTRY_TYPE_ACTUATOR, "Uint16 property", "ratio",
               yoriito::ValueType(static_cast<uint16_t>(0)),
               yoriito::ValueType(static_cast<uint16_t>(65535)), {}),
      MakeCase("Vehicle.Test.AttributeUint16Value", DataType::DATA_TYPE_UINT16,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Uint16 property", "ratio",
               yoriito::ValueType(static_cast<uint16_t>(0)),
               yoriito::ValueType(static_cast<uint16_t>(65535)), {}),

      MakeCase("Vehicle.Test.SensorUint32Value", DataType::DATA_TYPE_UINT32,
               EntryType::ENTRY_TYPE_SENSOR, "Uint32 property", "ratio",
               yoriito::ValueType(static_cast<uint32_t>(0)),
               yoriito::ValueType(static_cast<uint32_t>(4294967295u)), {}),
      MakeCase("Vehicle.Test.ActuatorUint32Value", DataType::DATA_TYPE_UINT32,
               EntryType::ENTRY_TYPE_ACTUATOR, "Uint32 property", "ratio",
               yoriito::ValueType(static_cast<uint32_t>(0)),
               yoriito::ValueType(static_cast<uint32_t>(4294967295u)), {}),
      MakeCase("Vehicle.Test.AttributeUint32Value", DataType::DATA_TYPE_UINT32,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Uint32 property", "ratio",
               yoriito::ValueType(static_cast<uint32_t>(0)),
               yoriito::ValueType(static_cast<uint32_t>(4294967295u)), {}),

      MakeCase(
          "Vehicle.Test.SensorUint64Value", DataType::DATA_TYPE_UINT64,
          EntryType::ENTRY_TYPE_SENSOR, "Uint64 property", "ratio",
          yoriito::ValueType(static_cast<uint64_t>(0)),
          yoriito::ValueType(static_cast<uint64_t>(18446744073709551615ull)),
          {}),
      MakeCase(
          "Vehicle.Test.ActuatorUint64Value", DataType::DATA_TYPE_UINT64,
          EntryType::ENTRY_TYPE_ACTUATOR, "Uint64 property", "ratio",
          yoriito::ValueType(static_cast<uint64_t>(0)),
          yoriito::ValueType(static_cast<uint64_t>(18446744073709551615ull)),
          {}),
      MakeCase(
          "Vehicle.Test.AttributeUint64Value", DataType::DATA_TYPE_UINT64,
          EntryType::ENTRY_TYPE_ATTRIBUTE, "Uint64 property", "ratio",
          yoriito::ValueType(static_cast<uint64_t>(0)),
          yoriito::ValueType(static_cast<uint64_t>(18446744073709551615ull)),
          {}),

      MakeCase("Vehicle.Test.SensorFloatValue", DataType::DATA_TYPE_FLOAT,
               EntryType::ENTRY_TYPE_SENSOR, "Float property", "ratio",
               yoriito::ValueType(static_cast<float>(-1000.0)),
               yoriito::ValueType(static_cast<float>(1000.0)), {}),
      MakeCase("Vehicle.Test.ActuatorFloatValue", DataType::DATA_TYPE_FLOAT,
               EntryType::ENTRY_TYPE_ACTUATOR, "Float property", "ratio",
               yoriito::ValueType(static_cast<float>(-1000.0)),
               yoriito::ValueType(static_cast<float>(1000.0)), {}),
      MakeCase("Vehicle.Test.AttributeFloatValue", DataType::DATA_TYPE_FLOAT,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Float property", "ratio",
               yoriito::ValueType(static_cast<float>(-1000.0)),
               yoriito::ValueType(static_cast<float>(1000.0)), {}),

      MakeCase("Vehicle.Test.SensorDoubleValue", DataType::DATA_TYPE_DOUBLE,
               EntryType::ENTRY_TYPE_SENSOR, "Double property", "ratio",
               yoriito::ValueType(static_cast<double>(-1000000.0)),
               yoriito::ValueType(static_cast<double>(1000000.0)), {}),
      MakeCase("Vehicle.Test.ActuatorDoubleValue", DataType::DATA_TYPE_DOUBLE,
               EntryType::ENTRY_TYPE_ACTUATOR, "Double property", "ratio",
               yoriito::ValueType(static_cast<double>(-1000000.0)),
               yoriito::ValueType(static_cast<double>(1000000.0)), {}),
      MakeCase("Vehicle.Test.AttributeDoubleValue", DataType::DATA_TYPE_DOUBLE,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Double property", "ratio",
               yoriito::ValueType(static_cast<double>(-1000000.0)),
               yoriito::ValueType(static_cast<double>(1000000.0)), {}),

      MakeCase("Vehicle.Test.SensorBoolArrayValue",
               DataType::DATA_TYPE_BOOLEAN_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Boolean array property", "", std::monostate{}, std::monostate{},
               {}),
      MakeCase("Vehicle.Test.ActuatorBoolArrayValue",
               DataType::DATA_TYPE_BOOLEAN_ARRAY,
               EntryType::ENTRY_TYPE_ACTUATOR, "Boolean array property", "",
               std::monostate{}, std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeBoolArrayValue",
               DataType::DATA_TYPE_BOOLEAN_ARRAY,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Boolean array property", "",
               std::monostate{}, std::monostate{}, {}),

      MakeCase("Vehicle.Test.SensorStringArrayValue",
               DataType::DATA_TYPE_STRING_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "String array property", "", std::monostate{}, std::monostate{},
               {}, "^[A-Za-z0-9_]*$"),
      MakeCase("Vehicle.Test.ActuatorStringArrayValue",
               DataType::DATA_TYPE_STRING_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "String array property", "", std::monostate{}, std::monostate{},
               {}, "^[A-Za-z0-9_]*$"),
      MakeCase("Vehicle.Test.AttributeStringArrayValue",
               DataType::DATA_TYPE_STRING_ARRAY,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "String array property", "",
               std::monostate{}, std::monostate{}, {}, "^[A-Za-z0-9_]*$"),

      MakeCase("Vehicle.Test.SensorInt8ArrayValue",
               DataType::DATA_TYPE_INT8_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Int8 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.ActuatorInt8ArrayValue",
               DataType::DATA_TYPE_INT8_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "Int8 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeInt8ArrayValue",
               DataType::DATA_TYPE_INT8_ARRAY, EntryType::ENTRY_TYPE_ATTRIBUTE,
               "Int8 array property", "ratio", std::monostate{},
               std::monostate{}, {}),

      MakeCase("Vehicle.Test.SensorInt16ArrayValue",
               DataType::DATA_TYPE_INT16_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Int16 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.ActuatorInt16ArrayValue",
               DataType::DATA_TYPE_INT16_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "Int16 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeInt16ArrayValue",
               DataType::DATA_TYPE_INT16_ARRAY, EntryType::ENTRY_TYPE_ATTRIBUTE,
               "Int16 array property", "ratio", std::monostate{},
               std::monostate{}, {}),

      MakeCase("Vehicle.Test.SensorInt32ArrayValue",
               DataType::DATA_TYPE_INT32_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Int32 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.ActuatorInt32ArrayValue",
               DataType::DATA_TYPE_INT32_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "Int32 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeInt32ArrayValue",
               DataType::DATA_TYPE_INT32_ARRAY, EntryType::ENTRY_TYPE_ATTRIBUTE,
               "Int32 array property", "ratio", std::monostate{},
               std::monostate{}, {}),

      MakeCase("Vehicle.Test.SensorInt64ArrayValue",
               DataType::DATA_TYPE_INT64_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Int64 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.ActuatorInt64ArrayValue",
               DataType::DATA_TYPE_INT64_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "Int64 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeInt64ArrayValue",
               DataType::DATA_TYPE_INT64_ARRAY, EntryType::ENTRY_TYPE_ATTRIBUTE,
               "Int64 array property", "ratio", std::monostate{},
               std::monostate{}, {}),

      MakeCase("Vehicle.Test.SensorUint8ArrayValue",
               DataType::DATA_TYPE_UINT8_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Uint8 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.ActuatorUint8ArrayValue",
               DataType::DATA_TYPE_UINT8_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "Uint8 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeUint8ArrayValue",
               DataType::DATA_TYPE_UINT8_ARRAY, EntryType::ENTRY_TYPE_ATTRIBUTE,
               "Uint8 array property", "ratio", std::monostate{},
               std::monostate{}, {}),

      MakeCase("Vehicle.Test.SensorUint16ArrayValue",
               DataType::DATA_TYPE_UINT16_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Uint16 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.ActuatorUint16ArrayValue",
               DataType::DATA_TYPE_UINT16_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "Uint16 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeUint16ArrayValue",
               DataType::DATA_TYPE_UINT16_ARRAY,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Uint16 array property",
               "ratio", std::monostate{}, std::monostate{}, {}),

      MakeCase("Vehicle.Test.SensorUint32ArrayValue",
               DataType::DATA_TYPE_UINT32_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Uint32 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.ActuatorUint32ArrayValue",
               DataType::DATA_TYPE_UINT32_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "Uint32 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeUint32ArrayValue",
               DataType::DATA_TYPE_UINT32_ARRAY,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Uint32 array property",
               "ratio", std::monostate{}, std::monostate{}, {}),

      MakeCase("Vehicle.Test.SensorUint64ArrayValue",
               DataType::DATA_TYPE_UINT64_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Uint64 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.ActuatorUint64ArrayValue",
               DataType::DATA_TYPE_UINT64_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "Uint64 array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeUint64ArrayValue",
               DataType::DATA_TYPE_UINT64_ARRAY,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Uint64 array property",
               "ratio", std::monostate{}, std::monostate{}, {}),

      MakeCase("Vehicle.Test.SensorFloatArrayValue",
               DataType::DATA_TYPE_FLOAT_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Float array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.ActuatorFloatArrayValue",
               DataType::DATA_TYPE_FLOAT_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "Float array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeFloatArrayValue",
               DataType::DATA_TYPE_FLOAT_ARRAY, EntryType::ENTRY_TYPE_ATTRIBUTE,
               "Float array property", "ratio", std::monostate{},
               std::monostate{}, {}),

      MakeCase("Vehicle.Test.SensorDoubleArrayValue",
               DataType::DATA_TYPE_DOUBLE_ARRAY, EntryType::ENTRY_TYPE_SENSOR,
               "Double array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.ActuatorDoubleArrayValue",
               DataType::DATA_TYPE_DOUBLE_ARRAY, EntryType::ENTRY_TYPE_ACTUATOR,
               "Double array property", "ratio", std::monostate{},
               std::monostate{}, {}),
      MakeCase("Vehicle.Test.AttributeDoubleArrayValue",
               DataType::DATA_TYPE_DOUBLE_ARRAY,
               EntryType::ENTRY_TYPE_ATTRIBUTE, "Double array property",
               "ratio", std::monostate{}, std::monostate{}, {}),
  };
}
}  // namespace

INSTANTIATE_TEST_SUITE_P(
    TestProducerInspect, ProducerInspect_WithParamTest,
    ::testing::ValuesIn(BuildPrimitiveSignalInspectParams()));

TEST(TestProducerInspect, StructAllTypeSignal) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  std::string target_path = "Vehicle.Test.StructAllTypeSignal";
  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  ASSERT_EQ(target_path, data.GetDataPoints()[0].GetPath());
  ASSERT_EQ(yoriito::databroker::BaseInspectData::EntryType::ENTRY_TYPE_SENSOR,
            data.GetDataPoints()[0].GetEntryType());
  ASSERT_EQ(yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_STRUCT,
            data.GetDataPoints()[0].GetDataType());
  ASSERT_EQ("Test Struct Signal", data.GetDataPoints()[0].GetDescription());

  const auto &data_type_ref = data.GetDataPoints()[0].GetDataTypeRef();
  ASSERT_TRUE(data_type_ref.has_value());
  ASSERT_TRUE(data_type_ref->IsResolved());
  auto root_struct = data_type_ref->Resolved().get();
  ASSERT_NE(root_struct, nullptr);

  const auto &root_properties = root_struct->GetProperties();
  ASSERT_EQ(root_properties.size(), 25);

  const std::map<std::string, yoriito::databroker::DataType> expected_types = {
      {"BoolValue", yoriito::databroker::DataType::DATA_TYPE_BOOLEAN},
      {"StringValue", yoriito::databroker::DataType::DATA_TYPE_STRING},
      {"Int8Value", yoriito::databroker::DataType::DATA_TYPE_INT8},
      {"Int16Value", yoriito::databroker::DataType::DATA_TYPE_INT16},
      {"Int32Value", yoriito::databroker::DataType::DATA_TYPE_INT32},
      {"Int64Value", yoriito::databroker::DataType::DATA_TYPE_INT64},
      {"Uint8Value", yoriito::databroker::DataType::DATA_TYPE_UINT8},
      {"Uint16Value", yoriito::databroker::DataType::DATA_TYPE_UINT16},
      {"Uint32Value", yoriito::databroker::DataType::DATA_TYPE_UINT32},
      {"Uint64Value", yoriito::databroker::DataType::DATA_TYPE_UINT64},
      {"FloatValue", yoriito::databroker::DataType::DATA_TYPE_FLOAT},
      {"DoubleValue", yoriito::databroker::DataType::DATA_TYPE_DOUBLE},
      {"BoolArrayValue",
       yoriito::databroker::DataType::DATA_TYPE_BOOLEAN_ARRAY},
      {"StringArrayValue",
       yoriito::databroker::DataType::DATA_TYPE_STRING_ARRAY},
      {"Int8ArrayValue", yoriito::databroker::DataType::DATA_TYPE_INT8_ARRAY},
      {"Int16ArrayValue", yoriito::databroker::DataType::DATA_TYPE_INT16_ARRAY},
      {"Int32ArrayValue", yoriito::databroker::DataType::DATA_TYPE_INT32_ARRAY},
      {"Int64ArrayValue", yoriito::databroker::DataType::DATA_TYPE_INT64_ARRAY},
      {"Uint8ArrayValue", yoriito::databroker::DataType::DATA_TYPE_UINT8_ARRAY},
      {"Uint16ArrayValue",
       yoriito::databroker::DataType::DATA_TYPE_UINT16_ARRAY},
      {"Uint32ArrayValue",
       yoriito::databroker::DataType::DATA_TYPE_UINT32_ARRAY},
      {"Uint64ArrayValue",
       yoriito::databroker::DataType::DATA_TYPE_UINT64_ARRAY},
      {"FloatArrayValue", yoriito::databroker::DataType::DATA_TYPE_FLOAT_ARRAY},
      {"DoubleArrayValue",
       yoriito::databroker::DataType::DATA_TYPE_DOUBLE_ARRAY},
      {"LocationValue", yoriito::databroker::DataType::DATA_TYPE_STRUCT},
  };

  const std::map<std::string, std::string> expected_descriptions = {
      {"BoolValue", "Boolean property"},
      {"StringValue", "String property"},
      {"Int8Value", "Int8 property"},
      {"Int16Value", "Int16 property"},
      {"Int32Value", "Int32 property"},
      {"Int64Value", "Int64 property"},
      {"Uint8Value", "Uint8 property"},
      {"Uint16Value", "Uint16 property"},
      {"Uint32Value", "Uint32 property"},
      {"Uint64Value", "Uint64 property"},
      {"FloatValue", "Float property"},
      {"DoubleValue", "Double property"},
      {"BoolArrayValue", "Boolean array property"},
      {"StringArrayValue", "String array property"},
      {"Int8ArrayValue", "Int8 array property"},
      {"Int16ArrayValue", "Int16 array property"},
      {"Int32ArrayValue", "Int32 array property"},
      {"Int64ArrayValue", "Int64 array property"},
      {"Uint8ArrayValue", "Uint8 array property"},
      {"Uint16ArrayValue", "Uint16 array property"},
      {"Uint32ArrayValue", "Uint32 array property"},
      {"Uint64ArrayValue", "Uint64 array property"},
      {"FloatArrayValue", "Float array property"},
      {"DoubleArrayValue", "Double array property"},
      {"LocationValue", "Nested location struct property"},
  };

  const std::set<std::string> unit_ratio_properties = {
      "Int8Value",        "Int16Value",       "Int32Value",
      "Int64Value",       "Uint8Value",       "Uint16Value",
      "Uint32Value",      "Uint64Value",      "FloatValue",
      "DoubleValue",      "Int8ArrayValue",   "Int16ArrayValue",
      "Int32ArrayValue",  "Int64ArrayValue",  "Uint8ArrayValue",
      "Uint16ArrayValue", "Uint32ArrayValue", "Uint64ArrayValue",
      "FloatArrayValue",  "DoubleArrayValue"};

  const auto empty_value = yoriito::ValueType(std::monostate{});

  std::set<std::string> observed_properties;
  bool location_struct_checked = false;
  for (const auto &prop : root_properties) {
    auto expected_it = expected_types.find(prop.GetPath());
    ASSERT_NE(expected_it, expected_types.end());
    ASSERT_EQ(prop.GetDataType(), expected_it->second);

    auto desc_it = expected_descriptions.find(prop.GetPath());
    ASSERT_NE(desc_it, expected_descriptions.end());
    ASSERT_EQ(prop.GetDescription(), desc_it->second);
    ASSERT_EQ(prop.GetComment(), "metadata test");
    ASSERT_EQ(prop.GetDeprecation(), "");
    ASSERT_EQ(prop.GetArraySize(), 0);

    if (unit_ratio_properties.count(prop.GetPath()) > 0) {
      ASSERT_EQ(prop.GetUnit(), "ratio");
    } else {
      ASSERT_EQ(prop.GetUnit(), "");
    }

    observed_properties.insert(prop.GetPath());

    if (prop.GetPath() == "LocationValue") {
      const auto &nested_ref = prop.GetDataTypeRef();
      ASSERT_TRUE(nested_ref.IsResolved());
      auto nested_struct = nested_ref.Resolved().get();
      ASSERT_NE(nested_struct, nullptr);
      ASSERT_EQ(nested_struct->GetPath(), "Types.TestDataType_Location");

      const auto &nested_properties = nested_struct->GetProperties();
      ASSERT_EQ(nested_properties.size(), 3);
      location_struct_checked = true;
    }
  }

  ASSERT_EQ(observed_properties.size(), expected_types.size());
  ASSERT_TRUE(location_struct_checked);
}

TEST(TestProducerInspect, NestedStruct) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  std::string target_path = "Vehicle.Test.NestedStructSignal";
  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  ASSERT_EQ(target_path, data.GetDataPoints()[0].GetPath());
  ASSERT_EQ(yoriito::databroker::BaseInspectData::EntryType::ENTRY_TYPE_SENSOR,
            data.GetDataPoints()[0].GetEntryType());
  ASSERT_EQ(yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_STRUCT,
            data.GetDataPoints()[0].GetDataType());
  ASSERT_EQ("Test Nested Struct Signal",
            data.GetDataPoints()[0].GetDescription());
  ASSERT_EQ("", data.GetDataPoints()[0].GetComment());
  ASSERT_EQ("", data.GetDataPoints()[0].GetUnit());

  const auto &allowed_values = data.GetDataPoints()[0].GetAllowed();
  const std::vector<std::string> expected_allowed_values{};
  ASSERT_EQ(expected_allowed_values, allowed_values);

  const auto &data_type_ref = data.GetDataPoints()[0].GetDataTypeRef();
  ASSERT_TRUE(data_type_ref.has_value());
  auto root_struct = data_type_ref->Resolved().get();
  ASSERT_NE(root_struct, nullptr);

  const auto &root_properties = root_struct->GetProperties();
  ASSERT_EQ(root_properties.size(), 2);

  bool has_property1 = false;
  bool has_property2 = false;
  for (const auto &prop : root_properties) {
    if (prop.GetPath() == "property1") {
      has_property1 = true;
      ASSERT_EQ(prop.GetDataType(),
                yoriito::databroker::DataType::DATA_TYPE_DOUBLE);
      ASSERT_EQ(prop.GetDescription(), "A primitive property");
    } else if (prop.GetPath() == "property2") {
      has_property2 = true;
      ASSERT_EQ(prop.GetDataType(),
                yoriito::databroker::DataType::DATA_TYPE_STRUCT);
      ASSERT_EQ(prop.GetDescription(), "A primitive property");

      const auto &nested_ref = prop.GetDataTypeRef();
      ASSERT_TRUE(nested_ref.IsResolved());
      auto nested_struct = nested_ref.Resolved().get();
      ASSERT_NE(nested_struct, nullptr);
      ASSERT_EQ(nested_struct->GetPath(), "Types.TestDataType_Location");

      const auto &nested_properties = nested_struct->GetProperties();
      ASSERT_EQ(nested_properties.size(), 3);

      bool has_latitude = false;
      bool has_longitude = false;
      bool has_altitude = false;
      for (const auto &nested_prop : nested_properties) {
        if (nested_prop.GetPath() == "Latitude") {
          has_latitude = true;
          ASSERT_EQ(nested_prop.GetDataType(),
                    yoriito::databroker::DataType::DATA_TYPE_DOUBLE);
          ASSERT_EQ(nested_prop.GetDescription(), "A primitive property");
        } else if (nested_prop.GetPath() == "Longitude") {
          has_longitude = true;
          ASSERT_EQ(nested_prop.GetDataType(),
                    yoriito::databroker::DataType::DATA_TYPE_DOUBLE);
          ASSERT_EQ(nested_prop.GetDescription(), "A primitive property");
        } else if (nested_prop.GetPath() == "Altitude") {
          has_altitude = true;
          ASSERT_EQ(nested_prop.GetDataType(),
                    yoriito::databroker::DataType::DATA_TYPE_DOUBLE);
          ASSERT_EQ(nested_prop.GetDescription(), "A primitive property");
        }
      }
      ASSERT_TRUE(has_latitude);
      ASSERT_TRUE(has_longitude);
      ASSERT_TRUE(has_altitude);
    }
  }

  ASSERT_TRUE(has_property1);
  ASSERT_TRUE(has_property2);
}

TEST(TestProducerInspect, StructSignalReturnsOnlyReferencedTypes) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  std::string target_path = "Vehicle.Test.StructSignal";
  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();

  std::set<std::string> struct_paths;
  for (const auto &struct_type : data.GetStructTypes()) {
    struct_paths.insert(struct_type->GetPath());
  }
  const std::set<std::string> expected_struct_paths{
      "Types.TestDataType_Location"};
  ASSERT_EQ(expected_struct_paths, struct_paths);
}

TEST(TestProducerInspect, NestedStructSignalReturnsOnlyReferencedTypes) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  std::string target_path = "Vehicle.Test.NestedStructSignal";
  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();

  std::set<std::string> struct_paths;
  for (const auto &struct_type : data.GetStructTypes()) {
    struct_paths.insert(struct_type->GetPath());
  }
  const std::set<std::string> expected_struct_paths{
      "Types.TestNestedStruct", "Types.TestDataType_Location"};
  ASSERT_EQ(expected_struct_paths, struct_paths);
}

TEST(TestProducerInspect, StructArraySignal) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  std::string target_path = "Vehicle.Test.StructArraySignal";
  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_EQ(1, data.GetDataPoints().size());
  ASSERT_EQ(target_path, data.GetDataPoints()[0].GetPath());
  ASSERT_EQ(yoriito::databroker::BaseInspectData::EntryType::ENTRY_TYPE_SENSOR,
            data.GetDataPoints()[0].GetEntryType());
  ASSERT_EQ(
      yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_STRUCT_ARRAY,
      data.GetDataPoints()[0].GetDataType());
  ASSERT_EQ("Test Struct Array Signal",
            data.GetDataPoints()[0].GetDescription());

  const auto &data_type_ref = data.GetDataPoints()[0].GetDataTypeRef();
  ASSERT_TRUE(data_type_ref.has_value());
  ASSERT_TRUE(data_type_ref->IsResolved());

  auto root_struct = data_type_ref->Resolved().get();
  ASSERT_NE(root_struct, nullptr);
  ASSERT_EQ(root_struct->GetPath(), "Types.TestDataType_Location");
  ASSERT_EQ(root_struct->GetProperties().size(), 3);
}

TEST(TestProducerInspect, NonStructSignalReturnsNoStructTypes) {
  yoriito::databroker::DataBrokerProducer *producer =
      yoriito::databroker::DataBrokerProducerFactory::GetInstance();

  std::string target_path = "Vehicle.Chassis.SteeringWheel.Angle";
  yoriito::Result<yoriito::databroker::ProducerInspectResponse> ret =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});

  ASSERT_TRUE(ret.IsOk());
  auto data = ret.TakeOk().value();
  ASSERT_TRUE(data.GetStructTypes().empty());
}
