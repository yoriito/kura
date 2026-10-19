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

#include "grpc/viss_grpc_inspect_util.h"

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common/inc/viss_common_error.h"
#include "grpc/viss_grpc_value_util.h"

namespace yoriito::databroker {

yoriito::databroker::BaseInspectData::EntryType
VissGrpcInspectUtil::ConvertEntryType(
    const yoriito::viss::v1::MetadataEntry &metadata) {
  switch (metadata.type()) {
    case yoriito::viss::v1::NodeType::NODE_ACTUATOR:
      return yoriito::databroker::BaseInspectData::EntryType::
          ENTRY_TYPE_ACTUATOR;
    case yoriito::viss::v1::NodeType::NODE_SENSOR:
      return yoriito::databroker::BaseInspectData::EntryType::ENTRY_TYPE_SENSOR;
    case yoriito::viss::v1::NodeType::NODE_ATTRIBUTE:
      return yoriito::databroker::BaseInspectData::EntryType::
          ENTRY_TYPE_ATTRIBUTE;
    default:
      return yoriito::databroker::BaseInspectData::EntryType::
          ENTRY_TYPE_UNSPECIFIED;
  }
}

yoriito::databroker::BaseInspectData::DataType
VissGrpcInspectUtil::ConvertDataType(yoriito::viss::v1::DataType data_type) {
  switch (data_type) {
    case yoriito::viss::v1::DataType::DATATYPE_NONE:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_UNSPECIFIED;
    case yoriito::viss::v1::DataType::DATATYPE_STRING:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_STRING;
    case yoriito::viss::v1::DataType::DATATYPE_BOOL:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_BOOLEAN;
    case yoriito::viss::v1::DataType::DATATYPE_INT8:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT8;
    case yoriito::viss::v1::DataType::DATATYPE_INT16:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT16;
    case yoriito::viss::v1::DataType::DATATYPE_INT32:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT32;
    case yoriito::viss::v1::DataType::DATATYPE_INT64:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT64;
    case yoriito::viss::v1::DataType::DATATYPE_UINT8:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT8;
    case yoriito::viss::v1::DataType::DATATYPE_UINT16:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT16;
    case yoriito::viss::v1::DataType::DATATYPE_UINT32:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT32;
    case yoriito::viss::v1::DataType::DATATYPE_UINT64:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT64;
    case yoriito::viss::v1::DataType::DATATYPE_FLOAT:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_FLOAT;
    case yoriito::viss::v1::DataType::DATATYPE_DOUBLE:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_DOUBLE;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_STRING:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_STRING_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_BOOL:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_BOOLEAN_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_INT8:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_INT8_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_INT16:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_INT16_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_INT32:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_INT32_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_INT64:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_INT64_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_UINT8:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_UINT8_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_UINT16:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_UINT16_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_UINT32:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_UINT32_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_UINT64:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_UINT64_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_FLOAT:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_FLOAT_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_ARRAY_DOUBLE:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_DOUBLE_ARRAY;
    case yoriito::viss::v1::DataType::DATATYPE_STRUCT:
      return yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_STRUCT;
    case yoriito::viss::v1::DataType::DATATYPE_STRUCT_ARRAY:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_STRUCT_ARRAY;
    default:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_UNSPECIFIED;
  }
}

yoriito::ValueType VissGrpcInspectUtil::ConvertMaxValueData(
    const yoriito::viss::v1::MetadataEntry &metadata) {
  ValueType max = std::monostate{};
  if (metadata.has_max()) {
    max = VissGrpcValueUtil::ConvertValueType(metadata.max());
  }
  return max;
}

yoriito::ValueType VissGrpcInspectUtil::ConvertMinValueData(
    const yoriito::viss::v1::MetadataEntry &metadata) {
  ValueType min = std::monostate{};
  if (metadata.has_min()) {
    min = VissGrpcValueUtil::ConvertValueType(metadata.min());
  }
  return min;
}

Result<BaseInspectData> VissGrpcInspectUtil::ConvertInspectData(
    const yoriito::viss::v1::MetadataEntry &metadata) {
  std::vector<std::string> allowed_values;
  const std::string path = metadata.path();
  const std::string comment = metadata.comment();
  const std::string deprecation = metadata.deprecated();
  const std::string unit = metadata.unit();
  std::string pattern = "";
  if (metadata.has_pattern() && (metadata.pattern().values_size() > 0)) {
    pattern = metadata.pattern().values(0);
  }

  auto builder = yoriito::databroker::BaseInspectData::CreateBuilder();

  auto entry_type = ConvertEntryType(metadata);
  auto data_type = ConvertDataType(metadata.datatype());

  ValueType max = ConvertMaxValueData(metadata);
  ValueType min = ConvertMinValueData(metadata);

  const std::string description = metadata.description();

  auto allowed = metadata.allowed();
  for (const auto &allow : allowed.values()) {
    allowed_values.push_back(allow);
  }

  auto default_value = VissGrpcValueUtil::ConvertValueType(metadata.default_());

  (void)builder.SetPath(path)
      .SetDataType(data_type)
      .SetEntryType(entry_type)
      .SetDescription(description)
      .SetComment(comment)
      .SetArraySize(metadata.arraysize())
      .SetDeprecation(deprecation)
      .SetDefaultValue(std::move(default_value))
      .SetAllowed(std::move(allowed_values))
      .SetMax(max)
      .SetMin(min)
      .SetUnit(unit)
      .SetPattern(pattern)
      .SetCustomAttributes(ConvertCustomMetadata(metadata.custom_metadata()));
  // pattern and custom attributes are not supported by the kura.
  // but implement for future use.

  return Result<BaseInspectData>::Ok(builder.Build());
}
// EntryType
yoriito::viss::v1::NodeType VissGrpcInspectUtil::ConvertEntryType(
    const yoriito::databroker::BaseInspectData::EntryType &entry_type) {
  if (entry_type ==
      yoriito::databroker::BaseInspectData::EntryType::ENTRY_TYPE_ACTUATOR) {
    return yoriito::viss::v1::NodeType::NODE_ACTUATOR;
  } else if (entry_type == yoriito::databroker::BaseInspectData::EntryType::
                               ENTRY_TYPE_SENSOR) {
    return yoriito::viss::v1::NodeType::NODE_SENSOR;
  } else if (entry_type == yoriito::databroker::BaseInspectData::EntryType::
                               ENTRY_TYPE_ATTRIBUTE) {
    return yoriito::viss::v1::NodeType::NODE_ATTRIBUTE;
  } else {
    return yoriito::viss::v1::NodeType::NODE_UNKNOWN;
  }
}
// DataTyoe
yoriito::viss::v1::DataType VissGrpcInspectUtil::ConvertDataType(
    const yoriito::databroker::BaseInspectData::DataType &data_type) {
  if (data_type ==
      yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_STRING) {
    return yoriito::viss::v1::DataType::DATATYPE_STRING;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_BOOLEAN) {
    return yoriito::viss::v1::DataType::DATATYPE_BOOL;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT8) {
    return yoriito::viss::v1::DataType::DATATYPE_INT8;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT16) {
    return yoriito::viss::v1::DataType::DATATYPE_INT16;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT32) {
    return yoriito::viss::v1::DataType::DATATYPE_INT32;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT64) {
    return yoriito::viss::v1::DataType::DATATYPE_INT64;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT8) {
    return yoriito::viss::v1::DataType::DATATYPE_UINT8;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT16) {
    return yoriito::viss::v1::DataType::DATATYPE_UINT16;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT32) {
    return yoriito::viss::v1::DataType::DATATYPE_UINT32;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT64) {
    return yoriito::viss::v1::DataType::DATATYPE_UINT64;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_FLOAT) {
    return yoriito::viss::v1::DataType::DATATYPE_FLOAT;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_DOUBLE) {
    return yoriito::viss::v1::DataType::DATATYPE_DOUBLE;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_STRING_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_STRING;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_BOOLEAN_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_BOOL;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_INT8_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_INT8;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_INT16_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_INT16;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_INT32_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_INT32;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_INT64_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_INT64;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_UINT8_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_UINT8;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_UINT16_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_UINT16;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_UINT32_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_UINT32;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_UINT64_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_UINT64;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_FLOAT_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_FLOAT;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_DOUBLE_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_ARRAY_DOUBLE;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_STRUCT) {
    return yoriito::viss::v1::DataType::DATATYPE_STRUCT;
  } else if (data_type == yoriito::databroker::BaseInspectData::DataType::
                              DATA_TYPE_STRUCT_ARRAY) {
    return yoriito::viss::v1::DataType::DATATYPE_STRUCT_ARRAY;
  } else {
    return yoriito::viss::v1::DataType::DATATYPE_NONE;
  }
  return yoriito::viss::v1::DataType::DATATYPE_NONE;
}

Result<yoriito::databroker::StructType> VissGrpcInspectUtil::ConvertStructData(
    const yoriito::viss::v1::StructMetadataEntry &metadata) {
  yoriito::databroker::StructType struct_type(metadata.path());
  struct_type.SetDescription(metadata.description());
  struct_type.SetDeprecation(metadata.deprecated());
  struct_type.SetComment(metadata.comment());

  auto custom_metadata = ConvertCustomMetadata(metadata.custom_metadata());
  for (auto &[key, value] : custom_metadata) {
    struct_type.AddCustomAttribute(key, std::move(value));
  }

  for (const auto &property : metadata.properties()) {
    auto ret = ConvertPropertyData(property);
    if (!ret.IsOk()) {
      return Result<yoriito::databroker::StructType>::Err(
          VissCommonError::CreateIllegalError("value type error"));
    }
    auto property_type = ret.TakeOk();
    if (!property_type.has_value()) {
      return Result<yoriito::databroker::StructType>::Err(
          VissCommonError::CreateIllegalError("value type error"));
    }
    struct_type.AddProperty(std::move(property_type.value()));
  }
  return Result<yoriito::databroker::StructType>::Ok(std::move(struct_type));
}

Result<yoriito::databroker::PropertyType>
VissGrpcInspectUtil::ConvertPropertyData(
    const yoriito::viss::v1::PropertyMetadataEntry &metadata) {
  yoriito::databroker::PropertyType property_type(
      metadata.name(), ConvertDataType(metadata.datatype()));
  property_type.SetDescription(metadata.description());
  property_type.SetDeprecation(metadata.deprecated());
  property_type.SetUnit(metadata.unit());
  property_type.SetMax(VissGrpcValueUtil::ConvertValueType(metadata.max()));
  property_type.SetMin(VissGrpcValueUtil::ConvertValueType(metadata.min()));
  property_type.SetComment(metadata.comment());
  std::vector<std::string> allowed_values;
  allowed_values.reserve(metadata.allowed().values_size());
  const auto &values = metadata.allowed().values();
  for (const auto &allowed : values) {
    allowed_values.push_back(allowed);
  }
  property_type.SetAllowed(std::move(allowed_values));
  property_type.SetArraySize(metadata.arraysize());
  property_type.SetCustomAttributes(
      ConvertCustomMetadata(metadata.custom_metadata()));

  return Result<yoriito::databroker::PropertyType>::Ok(
      std::move(property_type));
}

bool VissGrpcInspectUtil::IsStructDataType(
    yoriito::databroker::BaseInspectData::DataType data_type) {
  return (data_type ==
          yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_STRUCT) ||
         (data_type == yoriito::databroker::BaseInspectData::DataType::
                           DATA_TYPE_STRUCT_ARRAY);
}

void VissGrpcInspectUtil::BuildAndResolveStructTypeMap(
    const yoriito::viss::v1::InspectResponse &response,
    StructTypeMap &struct_type_map) {
  // 1st phase: build all struct types first.
  const auto &struct_packages = response.metadata().struct_metadata_entries();
  for (const auto &struct_package : struct_packages) {
    auto ret = VissGrpcInspectUtil::ConvertStructData(struct_package);
    if (ret.IsOk()) {
      struct_type_map[struct_package.path()] =
          std::make_shared<yoriito::databroker::StructType>(
              ret.TakeOk().value());
    }
  }

  // 2nd phase: resolve references between already-built struct types.
  for (const auto &struct_package : struct_packages) {
    auto target_struct_it = struct_type_map.find(struct_package.path());
    if (target_struct_it == struct_type_map.end()) {
      continue;
    }

    for (const auto &property_package : struct_package.properties()) {
      auto data_type =
          VissGrpcInspectUtil::ConvertDataType(property_package.datatype());
      if (!IsStructDataType(data_type) ||
          property_package.datatype_path().empty()) {
        continue;
      }

      auto nested_struct_it =
          struct_type_map.find(property_package.datatype_path());
      if (nested_struct_it == struct_type_map.end()) {
        continue;
      }

      auto *target_property =
          target_struct_it->second->GetMutableProperty(property_package.name());
      if (target_property == nullptr) {
        continue;
      }

      target_property->SetStructTypeRef(nested_struct_it->second);
    }
  }
}
std::map<std::string, std::string> VissGrpcInspectUtil::ConvertCustomMetadata(
    const google::protobuf::Struct &custom_metadata) {
  std::map<std::string, std::string> custom_attributes;
  for (const auto &[key, value] : custom_metadata.fields()) {
    std::string value_str;
    switch (value.kind_case()) {
      case google::protobuf::Value::kStringValue:
        value_str = value.string_value();
        break;
      case google::protobuf::Value::kNumberValue:
        value_str = std::to_string(value.number_value());
        break;
      case google::protobuf::Value::kBoolValue:
        value_str = value.bool_value() ? "true" : "false";
        break;
      case google::protobuf::Value::kNullValue:
        value_str = "null";
        break;
      case google::protobuf::Value::kStructValue:
      case google::protobuf::Value::kListValue:
        value_str = value.DebugString();
        break;
      case google::protobuf::Value::KIND_NOT_SET:
        break;
      default:
        value_str = value.DebugString();
        break;
    }
    (void)custom_attributes.emplace(key, std::move(value_str));
  }
  return custom_attributes;
}

}  // namespace yoriito::databroker
