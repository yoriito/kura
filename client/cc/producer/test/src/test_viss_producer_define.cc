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
#include "databroker/types/property_type.h"
#include "databroker/types/struct_type.h"
#include "producer/inc/viss_producer.h"
#include "yoriito/yoriito_log.h"

class TestProducerDefine : public testing::Test {
 protected:
  void SetUp() override {
    producer = yoriito::databroker::DataBrokerProducerFactory::GetInstance();
  }
  void TearDown() override {}

  yoriito::databroker::DataBrokerProducer *producer;
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
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});
  ASSERT_TRUE(ret.IsOk());
  auto inspect_response = ret.TakeOk().value();
  ASSERT_EQ(1, inspect_response.GetDataPoints().size());
  auto inspect_data = inspect_response.GetDataPoints()[0];
  ASSERT_EQ(target_path, inspect_data.GetPath());
  ASSERT_EQ(
      yoriito::databroker::ProducerDefineEntry::DataType::DATA_TYPE_UINT16,
      inspect_data.GetDataType());  //  DataType data_type,
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
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});
  ASSERT_TRUE(ret.IsOk());
}

TEST_F(TestProducerDefine, DefineSimpleStructAndInspect) {
  const std::string target_path = "Vehicle.Test.SimpleStructSignal";

  auto simple_struct =
      std::make_shared<yoriito::databroker::StructType>("Types.SimpleStruct");
  yoriito::databroker::PropertyType value_property(
      "Value", yoriito::databroker::DataType::DATA_TYPE_UINT8);
  value_property.SetDescription("Simple value property");
  simple_struct->AddProperty(std::move(value_property));

  auto builder = yoriito::databroker::BaseInspectData::CreateBuilder();
  builder.SetPath(target_path)
      .SetDataType(yoriito::databroker::DataType::DATA_TYPE_STRUCT)
      .SetEntryType(
          yoriito::databroker::BaseInspectData::EntryType::ENTRY_TYPE_SENSOR)
      .SetDescription("Simple struct signal")
      .SetComment("")
      .SetDeprecation("")
      .SetUnit("")
      .SetMin(yoriito::ValueType())
      .SetMax(yoriito::ValueType())
      .SetAllowed(std::vector<std::string>{})
      .SetDefaultValue(yoriito::ValueType())
      .SetArraySize(0)
      .SetPattern("")
      .SetCustomAttributes({});

  auto define_entry =
      yoriito::databroker::ProducerDefineEntry::CreateFromBuilder(builder);
  define_entry.SetDataTypeRef(
      yoriito::databroker::StructTypeRef("Types.SimpleStruct"));

  std::vector<yoriito::databroker::ProducerDefineEntry> define_entries;
  define_entries.push_back(define_entry);
  std::vector<std::shared_ptr<yoriito::databroker::StructType>> struct_types;
  struct_types.push_back(simple_struct);

  yoriito::databroker::ProducerDefineRequest request(std::move(define_entries),
                                                     std::move(struct_types));

  yoriito::Result<void> define_result = producer->Define(request);
  ASSERT_TRUE(define_result.IsOk());

  yoriito::Result<yoriito::databroker::ProducerInspectResponse> inspect_result =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});
  ASSERT_TRUE(inspect_result.IsOk());

  auto inspect_response = inspect_result.TakeOk().value();
  ASSERT_EQ(1, inspect_response.GetDataPoints().size());
  auto inspect_data = inspect_response.GetDataPoints()[0];
  ASSERT_EQ(target_path, inspect_data.GetPath());
  ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_STRUCT,
            inspect_data.GetDataType());

  const auto &entries = request.GetDefineEntries();
  ASSERT_EQ(1, entries.size());
  ASSERT_EQ(target_path, entries[0].GetPath());
  ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_STRUCT,
            entries[0].GetDataType());
  ASSERT_TRUE(entries[0].GetDataTypeRef().has_value());
  ASSERT_EQ("Types.SimpleStruct", entries[0].GetDataTypeRef()->GetPath());

  const auto &types = request.GetStructTypes();
  ASSERT_EQ(1, types.size());
  ASSERT_EQ("Types.SimpleStruct", types[0]->GetPath());
  ASSERT_EQ(1, types[0]->GetProperties().size());
  const auto *value_property_view = types[0]->GetProperties().at("Value");
  ASSERT_NE(value_property_view, nullptr);
  ASSERT_EQ("Value", value_property_view->GetPath());
  ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_UINT8,
            value_property_view->GetDataType());

  // foreach property in the struct type, check its details
  {
    auto properties = types[0]->GetProperties();
    for (const auto &property : properties) {
      if (property.GetPath() == "Value") {
        ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_UINT8,
                  property.GetDataType());
        ASSERT_EQ("Simple value property", property.GetDescription());
      }
    }
  }
}

TEST_F(TestProducerDefine, DefineStructWithMultiplePropertiesAndInspect) {
  const std::string target_path = "Vehicle.Test.MultiPropertyStructSignal";

  auto multi_struct = std::make_shared<yoriito::databroker::StructType>(
      "Types.MultiPropertyStruct");

  yoriito::databroker::PropertyType speed_property(
      "Speed", yoriito::databroker::DataType::DATA_TYPE_UINT16);
  speed_property.SetDescription("Vehicle speed");
  multi_struct->AddProperty(std::move(speed_property));

  yoriito::databroker::PropertyType enabled_property(
      "Enabled", yoriito::databroker::DataType::DATA_TYPE_BOOLEAN);
  enabled_property.SetDescription("Enable flag");
  multi_struct->AddProperty(std::move(enabled_property));

  yoriito::databroker::PropertyType mode_property(
      "Mode", yoriito::databroker::DataType::DATA_TYPE_STRING);
  mode_property.SetDescription("Drive mode");
  multi_struct->AddProperty(std::move(mode_property));

  auto builder = yoriito::databroker::BaseInspectData::CreateBuilder();
  builder.SetPath(target_path)
      .SetDataType(yoriito::databroker::DataType::DATA_TYPE_STRUCT)
      .SetEntryType(
          yoriito::databroker::BaseInspectData::EntryType::ENTRY_TYPE_SENSOR)
      .SetDescription("Multi property struct signal")
      .SetComment("")
      .SetDeprecation("")
      .SetUnit("")
      .SetMin(yoriito::ValueType())
      .SetMax(yoriito::ValueType())
      .SetAllowed(std::vector<std::string>{})
      .SetDefaultValue(yoriito::ValueType())
      .SetArraySize(0)
      .SetPattern("")
      .SetCustomAttributes({});

  auto define_entry =
      yoriito::databroker::ProducerDefineEntry::CreateFromBuilder(builder);
  define_entry.SetDataTypeRef(
      yoriito::databroker::StructTypeRef("Types.MultiPropertyStruct"));

  std::vector<yoriito::databroker::ProducerDefineEntry> define_entries;
  define_entries.push_back(define_entry);
  std::vector<std::shared_ptr<yoriito::databroker::StructType>> struct_types;
  struct_types.push_back(multi_struct);

  yoriito::databroker::ProducerDefineRequest request(std::move(define_entries),
                                                     std::move(struct_types));

  yoriito::Result<void> define_result = producer->Define(request);
  ASSERT_TRUE(define_result.IsOk());

  yoriito::Result<yoriito::databroker::ProducerInspectResponse> inspect_result =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});
  ASSERT_TRUE(inspect_result.IsOk());

  auto inspect_response = inspect_result.TakeOk().value();
  ASSERT_EQ(1, inspect_response.GetDataPoints().size());
  auto inspect_data = inspect_response.GetDataPoints()[0];
  ASSERT_EQ(target_path, inspect_data.GetPath());
  ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_STRUCT,
            inspect_data.GetDataType());

  const auto &data_type_ref = inspect_data.GetDataTypeRef();
  ASSERT_TRUE(data_type_ref.has_value());
  ASSERT_TRUE(data_type_ref->IsResolved());
  auto inspected_struct = data_type_ref->Resolved().get();
  ASSERT_NE(inspected_struct, nullptr);
  ASSERT_EQ("Types.MultiPropertyStruct", inspected_struct->GetPath());

  const auto &properties = inspected_struct->GetProperties();
  ASSERT_EQ(3, properties.size());

  bool has_speed = false;
  bool has_enabled = false;
  bool has_mode = false;
  for (const auto &property : properties) {
    if (property.GetPath() == "Speed") {
      has_speed = true;
      ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_UINT16,
                property.GetDataType());
      ASSERT_EQ("Vehicle speed", property.GetDescription());
    } else if (property.GetPath() == "Enabled") {
      has_enabled = true;
      ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_BOOLEAN,
                property.GetDataType());
      ASSERT_EQ("Enable flag", property.GetDescription());
    } else if (property.GetPath() == "Mode") {
      has_mode = true;
      ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_STRING,
                property.GetDataType());
      ASSERT_EQ("Drive mode", property.GetDescription());
    }
  }

  ASSERT_TRUE(has_speed);
  ASSERT_TRUE(has_enabled);
  ASSERT_TRUE(has_mode);
}

TEST_F(TestProducerDefine, DefineStructPropertyContainsStructAndInspect) {
  const std::string target_path = "Vehicle.Test.StructInStructSignal";

  auto child_struct = std::make_shared<yoriito::databroker::StructType>(
      "Types.ChildStructForDefine");
  yoriito::databroker::PropertyType latitude_property(
      "Latitude", yoriito::databroker::DataType::DATA_TYPE_DOUBLE);
  latitude_property.SetDescription("Latitude value");
  child_struct->AddProperty(std::move(latitude_property));

  yoriito::databroker::PropertyType longitude_property(
      "Longitude", yoriito::databroker::DataType::DATA_TYPE_DOUBLE);
  longitude_property.SetDescription("Longitude value");
  child_struct->AddProperty(std::move(longitude_property));

  auto parent_struct = std::make_shared<yoriito::databroker::StructType>(
      "Types.ParentStructForDefine");
  yoriito::databroker::PropertyType id_property(
      "Id", yoriito::databroker::DataType::DATA_TYPE_UINT32);
  id_property.SetDescription("Identifier");
  parent_struct->AddProperty(std::move(id_property));

  yoriito::databroker::PropertyType location_property(
      "Location", yoriito::databroker::DataType::DATA_TYPE_STRUCT,
      yoriito::databroker::StructTypeRef(child_struct));
  location_property.SetDescription("Nested location struct");
  parent_struct->AddProperty(std::move(location_property));

  auto builder = yoriito::databroker::BaseInspectData::CreateBuilder();
  builder.SetPath(target_path)
      .SetDataType(yoriito::databroker::DataType::DATA_TYPE_STRUCT)
      .SetEntryType(
          yoriito::databroker::BaseInspectData::EntryType::ENTRY_TYPE_SENSOR)
      .SetDescription("Struct property contains struct")
      .SetComment("")
      .SetDeprecation("")
      .SetUnit("")
      .SetMin(yoriito::ValueType())
      .SetMax(yoriito::ValueType())
      .SetAllowed(std::vector<std::string>{})
      .SetDefaultValue(yoriito::ValueType())
      .SetArraySize(0)
      .SetPattern("")
      .SetCustomAttributes({});

  auto define_entry =
      yoriito::databroker::ProducerDefineEntry::CreateFromBuilder(builder);
  define_entry.SetDataTypeRef(
      yoriito::databroker::StructTypeRef("Types.ParentStructForDefine"));

  std::vector<yoriito::databroker::ProducerDefineEntry> define_entries;
  define_entries.push_back(define_entry);
  std::vector<std::shared_ptr<yoriito::databroker::StructType>> struct_types;
  struct_types.push_back(parent_struct);
  struct_types.push_back(child_struct);

  yoriito::databroker::ProducerDefineRequest request(std::move(define_entries),
                                                     std::move(struct_types));

  yoriito::Result<void> define_result = producer->Define(request);
  ASSERT_TRUE(define_result.IsOk());

  yoriito::Result<yoriito::databroker::ProducerInspectResponse> inspect_result =
      producer->Inspect(yoriito::databroker::ProducerInspectRequest{
          std::string(target_path)});
  ASSERT_TRUE(inspect_result.IsOk());

  auto inspect_response = inspect_result.TakeOk().value();
  ASSERT_EQ(1, inspect_response.GetDataPoints().size());
  auto inspect_data = inspect_response.GetDataPoints()[0];
  ASSERT_EQ(target_path, inspect_data.GetPath());
  ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_STRUCT,
            inspect_data.GetDataType());

  const auto &root_ref = inspect_data.GetDataTypeRef();
  ASSERT_TRUE(root_ref.has_value());
  ASSERT_TRUE(root_ref->IsResolved());
  auto parent = root_ref->Resolved().get();
  ASSERT_NE(parent, nullptr);
  ASSERT_EQ("Types.ParentStructForDefine", parent->GetPath());

  const auto &parent_properties = parent->GetProperties();
  ASSERT_EQ(2, parent_properties.size());

  bool has_id = false;
  bool has_location = false;
  for (const auto &property : parent_properties) {
    if (property.GetPath() == "Id") {
      has_id = true;
      ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_UINT32,
                property.GetDataType());
      ASSERT_EQ("Identifier", property.GetDescription());
    } else if (property.GetPath() == "Location") {
      // TODO: DataTypeRef should be resolved for nested struct property
      has_location = true;
      ASSERT_EQ(yoriito::databroker::DataType::DATA_TYPE_STRUCT,
                property.GetDataType());
      ASSERT_FALSE(property.GetDataTypeRef().IsResolved());
    }
  }

  ASSERT_TRUE(has_id);
  ASSERT_TRUE(has_location);
}
