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

#include <string>
#include <vector>
//
#include "grpc/viss_grpc_inspect_util.h"
#include "viss_common_error.h"

namespace yoriito::databroker {

ValueType VissGrpcInspectUtil::ConvertValueData(
    yoriito::viss::v1::ValueType val) {
  ValueType ret;
  if (val.has_string()) {
    ret = ValueType(val.string());
  } else if (val.has_bool_()) {
    ret = ValueType(val.bool_());
  } else if (val.has_int32()) {
    ret = ValueType(static_cast<int32_t>(val.int32()));
  } else if (val.has_int64()) {
    ret = ValueType(static_cast<int64_t>(val.int64()));
  } else if (val.has_uint32()) {
    ret = ValueType(static_cast<uint32_t>(val.uint32()));
  } else if (val.has_uint64()) {
    ret = ValueType(static_cast<uint64_t>(val.uint64()));
  } else if (val.has_float_()) {
    ret = ValueType(static_cast<float>(val.float_()));
  } else if (val.has_double_()) {
    ret = ValueType(static_cast<double>(val.double_()));
  } else if (val.has_string_array()) {
    const auto& string_array = val.string_array();
    std::vector<std::string> retValue;
    for (int i = 0; i < string_array.values_size(); ++i) {
      retValue.push_back(string_array.values(i));
    }
    ret = ValueType(retValue);
  } else if (val.has_bool_array()) {
    const auto& bool_array = val.bool_array();
    std::vector<bool> retValue;
    for (int i = 0; i < bool_array.values_size(); ++i) {
      retValue.push_back(bool_array.values(i));
    }
    ret = ValueType(retValue);
  } else if (val.has_int32_array()) {
    const auto& int32_array = val.int32_array();
    std::vector<int32_t> retValue;
    for (int i = 0; i < int32_array.values_size(); ++i) {
      retValue.push_back(static_cast<int32_t>(int32_array.values(i)));
    }
    ret = ValueType(retValue);
  } else if (val.has_int64_array()) {
    const auto& int64_array = val.int64_array();
    std::vector<int64_t> retValue;
    for (int i = 0; i < int64_array.values_size(); ++i) {
      retValue.push_back(static_cast<int64_t>(int64_array.values(i)));
    }
    ret = ValueType(retValue);
  } else if (val.has_uint32_array()) {
    const auto& uint32_array = val.uint32_array();
    std::vector<uint32_t> retValue;
    for (int i = 0; i < uint32_array.values_size(); ++i) {
      retValue.push_back(static_cast<uint32_t>(uint32_array.values(i)));
    }
    ret = ValueType(retValue);
  } else if (val.has_uint64_array()) {
    const auto& uint64_array = val.uint64_array();
    std::vector<uint64_t> retValue;
    for (int i = 0; i < uint64_array.values_size(); ++i) {
      retValue.push_back(static_cast<uint64_t>(uint64_array.values(i)));
    }
    ret = ValueType(retValue);
  } else if (val.has_float_array()) {
    const auto& float_array = val.float_array();
    std::vector<float> retValue;
    for (int i = 0; i < float_array.values_size(); ++i) {
      retValue.push_back(static_cast<float>(float_array.values(i)));
    }
    ret = ValueType(retValue);
  } else if (val.has_double_array()) {
    const auto& double_array = val.double_array();
    std::vector<double> retValue;
    for (int i = 0; i < double_array.values_size(); ++i) {
      retValue.push_back(static_cast<double>(double_array.values(i)));
    }
    ret = ValueType(retValue);
  }

  return ret;
}
yoriito::databroker::BaseInspectData::EntryType
VissGrpcInspectUtil::ConvertEntryType(
    const yoriito::viss::v1::MetadataEntry& metadata) {
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
    default:
      return yoriito::databroker::BaseInspectData::DataType::
          DATA_TYPE_UNSPECIFIED;
  }
}

yoriito::ValueType VissGrpcInspectUtil::ConvertInt32ToValueData(
    int32_t value, yoriito::databroker::BaseInspectData::DataType dataType) {
  ValueType converted_value = std::monostate{};
  switch (dataType) {
    case yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT8:
      converted_value = static_cast<int8_t>(value);
      break;
    case yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT16:
      converted_value = static_cast<int16_t>(value);
      break;
    case yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT32:
      converted_value = static_cast<int32_t>(value);
      break;
    case yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_INT64:
      converted_value = static_cast<int64_t>(value);
      break;
    case yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT8:
      converted_value = static_cast<uint8_t>(value);
      break;
    case yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT16:
      converted_value = static_cast<uint16_t>(value);
      break;
    case yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT32:
      converted_value = static_cast<uint32_t>(value);
      break;
    case yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_UINT64:
      converted_value = static_cast<uint64_t>(value);
      break;
    case yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_FLOAT:
      converted_value = static_cast<float>(value);
      break;
    case yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_DOUBLE:
      converted_value = static_cast<double>(value);
      break;
    default:
      break;
  }
  return converted_value;
}

yoriito::ValueType VissGrpcInspectUtil::ConvertMaxValueData(
    const yoriito::viss::v1::MetadataEntry& metadata) {
  ValueType max = std::monostate{};
  auto data_type = ConvertDataType(metadata.datatype());
  if (metadata.has_max()) {
    max = ConvertInt32ToValueData(metadata.max(), data_type);
  }
  return max;
}

yoriito::ValueType VissGrpcInspectUtil::ConvertMinValueData(
    const yoriito::viss::v1::MetadataEntry& metadata) {
  ValueType min = std::monostate{};
  auto data_type = ConvertDataType(metadata.datatype());
  if (metadata.has_min()) {
    min = ConvertInt32ToValueData(metadata.min(), data_type);
  }
  return min;
}

Result<BaseInspectData> VissGrpcInspectUtil::ConvertInspectData(
    const yoriito::viss::v1::MetadataEntry& metadata) {
  std::vector<std::string> allowed_values;
  auto path = metadata.path();
  std::string comment = metadata.comment();
  auto deprecation = metadata.deprecated();
  std::string unit = metadata.unit();
  auto pattern = metadata.pattern();

  auto builder = yoriito::databroker::BaseInspectData::CreateBuilder();

  auto entry_type = ConvertEntryType(metadata);
  auto data_type = ConvertDataType(metadata.datatype());

  ValueType max = ConvertMaxValueData(metadata);
  ValueType min = ConvertMinValueData(metadata);

  auto description = metadata.description();

  auto allowed = metadata.allowed();
  for (const auto& allow : allowed.values()) {
    allowed_values.push_back(allow);
  }
  builder = builder.SetArraySize(metadata.arraysize());

  auto default_value = ConvertValueData(metadata.default_());
  // const auto& fields = metadata.custom_metadata().fields();

  builder.SetPath(path)
      .SetDataType(data_type)
      .SetEntryType(entry_type)
      .SetDescription(description)
      .SetComment(comment)
      .SetDeprecation(deprecation)
      .SetDefaultValue(std::move(default_value))
      .SetAllowed(std::move(allowed_values))
      .SetMax(max)
      .SetMin(min)
      .SetUnit(unit);

  return Result<BaseInspectData>::Ok(builder.Build());
}
// EntryType
yoriito::viss::v1::NodeType VissGrpcInspectUtil::ConvertEntryType(
    const yoriito::databroker::BaseInspectData::EntryType& entry_type) {
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
    const yoriito::databroker::BaseInspectData::DataType& data_type) {
  if (data_type ==
      yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_STRING) {
    return yoriito::viss::v1::DataType::DATATYPE_STRING;
  } else if (data_type ==
             yoriito::databroker::BaseInspectData::DataType::DATA_TYPE_BOOLEAN) {
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
  } else {
    return yoriito::viss::v1::DataType::DATATYPE_NONE;
  }
  return yoriito::viss::v1::DataType::DATATYPE_NONE;
}

// Convert DataType to int32
Result<int32_t> VissGrpcInspectUtil::ConvertDataTypeToInt(
    const ValueType& value) {
  if (std::holds_alternative<int8_t>(value)) {
    return Result<int32_t>::Ok(static_cast<int32_t>(std::get<int8_t>(value)));
  } else if (std::holds_alternative<int16_t>(value)) {
    return Result<int32_t>::Ok(static_cast<int32_t>(std::get<int16_t>(value)));
  } else if (std::holds_alternative<int32_t>(value)) {
    return Result<int32_t>::Ok(static_cast<int32_t>(std::get<int32_t>(value)));
  } else if (std::holds_alternative<uint8_t>(value)) {
    return Result<int32_t>::Ok(static_cast<int32_t>(std::get<uint8_t>(value)));
  } else if (std::holds_alternative<uint16_t>(value)) {
    return Result<int32_t>::Ok(static_cast<int32_t>(std::get<uint16_t>(value)));
  } else if (std::holds_alternative<uint32_t>(value)) {
    return Result<int32_t>::Ok(static_cast<int32_t>(std::get<uint32_t>(value)));
  }
  return Result<int32_t>::Err(
      VissCommonError::CreateIllegalError("value type error"));
}
}  // namespace yoriito::databroker
