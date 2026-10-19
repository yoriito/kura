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

#include "grpc/viss_grpc_value_util.h"

#include <string>
#include <type_traits>
#include <vector>

#include "common/inc/viss_common_error.h"

namespace yoriito::databroker {

template <typename T, typename Array>
ValueType VissGrpcValueUtil::ConvertArray(const Array &array) {
  std::vector<T> result;
  result.reserve(array.values_size());
  for (const auto &value : array.values()) {
    result.push_back(static_cast<T>(value));
  }
  return ValueType(std::move(result));
}

void VissGrpcValueUtil::SetValueObject(
    yoriito::viss::v1::ValueObject &value_object,
    const StructData &struct_data) {
  const auto &properties = struct_data.GetProperties();
  for (const auto &elem : properties) {
    auto *prop = value_object.add_properties();
    prop->set_key(std::string(elem.first));
    auto *mut_val = prop->mutable_value();
    (void)VissGrpcValueUtil::SetDataPointValue(*mut_val, elem.second);
  }
}

template <typename T, typename MutableArray>
void VissGrpcValueUtil::SetArrayValue(
    yoriito::viss::v1::ValueType &datapoint_value_type, const ValueType &value,
    MutableArray mutable_array) {
  auto *array = (datapoint_value_type.*mutable_array)();
  for (const auto &element : std::get<std::vector<T>>(value)) {
    array->add_values(
        static_cast<std::decay_t<decltype(array->values(0))>>(element));
  }
}

StructData VissGrpcValueUtil::ConvertValueObject(
    const yoriito::viss::v1::ValueObject &value_object) {
  StructData ret_value;

  for (const auto &prop : value_object.properties()) {
    ret_value.SetProperty(std::string(prop.key()),
                          VissGrpcValueUtil::ConvertValueType(prop.value()));
  }
  return ret_value;
}

ValueType VissGrpcValueUtil::ConvertValueObjectArray(
    const yoriito::viss::v1::ValueObjectArray &value_object_array) {
  StructDataArray result;
  result.reserve(value_object_array.values_size());
  for (const auto &value : value_object_array.values()) {
    result.push_back(std::make_unique<StructData>(ConvertValueObject(value)));
  }
  return ValueType(std::move(result));
}

// Notes on integer types in value.proto:
// in value.proto, integer types smaller than int32 are defined as sint32.
// Therefore, int8_t and int16_t are cast to int32_t when setting the value.

Result<void> VissGrpcValueUtil::SetDataPointValue(
    yoriito::viss::v1::ValueType &datapoint_value_type,
    const ValueType &value) {
  if (std::holds_alternative<std::string>(value)) {
    datapoint_value_type.set_string(std::get<std::string>(value));
  } else if (std::holds_alternative<bool>(value)) {
    datapoint_value_type.set_bool_(std::get<bool>(value));
  } else if (std::holds_alternative<int8_t>(value)) {
    datapoint_value_type.set_int8(
        static_cast<int32_t>(std::get<int8_t>(value)));
  } else if (std::holds_alternative<int16_t>(value)) {
    datapoint_value_type.set_int16(
        static_cast<int32_t>(std::get<int16_t>(value)));
  } else if (std::holds_alternative<int32_t>(value)) {
    datapoint_value_type.set_int32(std::get<int32_t>(value));
  } else if (std::holds_alternative<int64_t>(value)) {
    datapoint_value_type.set_int64(std::get<int64_t>(value));
  } else if (std::holds_alternative<uint8_t>(value)) {
    datapoint_value_type.set_uint8(
        static_cast<uint32_t>(std::get<uint8_t>(value)));
  } else if (std::holds_alternative<uint16_t>(value)) {
    datapoint_value_type.set_uint16(
        static_cast<uint32_t>(std::get<uint16_t>(value)));
  } else if (std::holds_alternative<uint32_t>(value)) {
    datapoint_value_type.set_uint32(std::get<uint32_t>(value));
  } else if (std::holds_alternative<uint64_t>(value)) {
    datapoint_value_type.set_uint64(std::get<uint64_t>(value));
  } else if (std::holds_alternative<float>(value)) {
    datapoint_value_type.set_float_(std::get<float>(value));
  } else if (std::holds_alternative<double>(value)) {
    datapoint_value_type.set_double_(std::get<double>(value));
  } else if (std::holds_alternative<std::vector<std::string>>(value)) {
    SetArrayValue<std::string>(
        datapoint_value_type, value,
        &yoriito::viss::v1::ValueType::mutable_string_array);
  } else if (std::holds_alternative<std::vector<bool>>(value)) {
    SetArrayValue<bool>(datapoint_value_type, value,
                        &yoriito::viss::v1::ValueType::mutable_bool_array);
  } else if (std::holds_alternative<std::vector<int8_t>>(value)) {
    SetArrayValue<int8_t>(datapoint_value_type, value,
                          &yoriito::viss::v1::ValueType::mutable_int8_array);
  } else if (std::holds_alternative<std::vector<int16_t>>(value)) {
    SetArrayValue<int16_t>(datapoint_value_type, value,
                           &yoriito::viss::v1::ValueType::mutable_int16_array);
  } else if (std::holds_alternative<std::vector<int32_t>>(value)) {
    SetArrayValue<int32_t>(datapoint_value_type, value,
                           &yoriito::viss::v1::ValueType::mutable_int32_array);
  } else if (std::holds_alternative<std::vector<int64_t>>(value)) {
    SetArrayValue<int64_t>(datapoint_value_type, value,
                           &yoriito::viss::v1::ValueType::mutable_int64_array);
  } else if (std::holds_alternative<std::vector<uint8_t>>(value)) {
    SetArrayValue<uint8_t>(datapoint_value_type, value,
                           &yoriito::viss::v1::ValueType::mutable_uint8_array);
  } else if (std::holds_alternative<std::vector<uint16_t>>(value)) {
    SetArrayValue<uint16_t>(
        datapoint_value_type, value,
        &yoriito::viss::v1::ValueType::mutable_uint16_array);
  } else if (std::holds_alternative<std::vector<uint32_t>>(value)) {
    SetArrayValue<uint32_t>(
        datapoint_value_type, value,
        &yoriito::viss::v1::ValueType::mutable_uint32_array);
  } else if (std::holds_alternative<std::vector<uint64_t>>(value)) {
    SetArrayValue<uint64_t>(
        datapoint_value_type, value,
        &yoriito::viss::v1::ValueType::mutable_uint64_array);
  } else if (std::holds_alternative<std::vector<float>>(value)) {
    SetArrayValue<float>(datapoint_value_type, value,
                         &yoriito::viss::v1::ValueType::mutable_float_array);
  } else if (std::holds_alternative<std::vector<double>>(value)) {
    SetArrayValue<double>(datapoint_value_type, value,
                          &yoriito::viss::v1::ValueType::mutable_double_array);
  } else if (std::holds_alternative<StructDataPtr>(value)) {
    return SetStructDataValue(datapoint_value_type,
                              std::get<StructDataPtr>(value));
  } else if (std::holds_alternative<StructDataArray>(value)) {
    return SetStructDataArrayValue(datapoint_value_type,
                                   std::get<StructDataArray>(value));
  } else {
    return Result<void>::Err(

        VissCommonError::CreateIllegalError("value type error"));
  }
  return Result<void>::Ok();
}

Result<void> VissGrpcValueUtil::SetStructDataValue(
    yoriito::viss::v1::ValueType &datapoint_value_type,
    const StructDataPtr &value) {
  if (!value) {
    return Result<void>::Err(
        VissCommonError::CreateIllegalError("value type error"));
  }
  SetValueObject(*datapoint_value_type.mutable_value_object(), *value);
  return Result<void>::Ok();
}

Result<void> VissGrpcValueUtil::SetStructDataArrayValue(
    yoriito::viss::v1::ValueType &datapoint_value_type,
    const StructDataArray &value) {
  auto *value_object_array = datapoint_value_type.mutable_value_object_array();
  for (const auto &element : value) {
    if (!element) {
      return Result<void>::Err(
          VissCommonError::CreateIllegalError("value type error"));
    }
    SetValueObject(*value_object_array->add_values(), *element);
  }
  return Result<void>::Ok();
}

ValueType VissGrpcValueUtil::ConvertValueType(
    yoriito::viss::v1::ValueType val) {
  switch (val.value_type_case()) {
    case yoriito::viss::v1::ValueType::kUint8:
      return ValueType(static_cast<uint8_t>(val.uint8()));
    case yoriito::viss::v1::ValueType::kUint16:
      return ValueType(static_cast<uint16_t>(val.uint16()));
    case yoriito::viss::v1::ValueType::kUint32:
      return ValueType(static_cast<uint32_t>(val.uint32()));
    case yoriito::viss::v1::ValueType::kUint64:
      return ValueType(static_cast<uint64_t>(val.uint64()));
    case yoriito::viss::v1::ValueType::kInt8:
      return ValueType(static_cast<int8_t>(val.int8()));
    case yoriito::viss::v1::ValueType::kInt16:
      return ValueType(static_cast<int16_t>(val.int16()));
    case yoriito::viss::v1::ValueType::kInt32:
      return ValueType(static_cast<int32_t>(val.int32()));
    case yoriito::viss::v1::ValueType::kInt64:
      return ValueType(static_cast<int64_t>(val.int64()));
    case yoriito::viss::v1::ValueType::kFloat:
      return ValueType(val.float_());
    case yoriito::viss::v1::ValueType::kDouble:
      return ValueType(val.double_());
    case yoriito::viss::v1::ValueType::kString:
      return ValueType(val.string());
    case yoriito::viss::v1::ValueType::kBool:
      return ValueType(val.bool_());
    case yoriito::viss::v1::ValueType::kUint8Array:
      return ConvertArray<uint8_t>(val.uint8_array());
    case yoriito::viss::v1::ValueType::kUint16Array:
      return ConvertArray<uint16_t>(val.uint16_array());
    case yoriito::viss::v1::ValueType::kUint32Array:
      return ConvertArray<uint32_t>(val.uint32_array());
    case yoriito::viss::v1::ValueType::kUint64Array:
      return ConvertArray<uint64_t>(val.uint64_array());
    case yoriito::viss::v1::ValueType::kInt8Array:
      return ConvertArray<int8_t>(val.int8_array());
    case yoriito::viss::v1::ValueType::kInt16Array:
      return ConvertArray<int16_t>(val.int16_array());
    case yoriito::viss::v1::ValueType::kInt32Array:
      return ConvertArray<int32_t>(val.int32_array());
    case yoriito::viss::v1::ValueType::kInt64Array:
      return ConvertArray<int64_t>(val.int64_array());
    case yoriito::viss::v1::ValueType::kFloatArray:
      return ConvertArray<float>(val.float_array());
    case yoriito::viss::v1::ValueType::kDoubleArray:
      return ConvertArray<double>(val.double_array());
    case yoriito::viss::v1::ValueType::kStringArray:
      return ConvertArray<std::string>(val.string_array());
    case yoriito::viss::v1::ValueType::kBoolArray:
      return ConvertArray<bool>(val.bool_array());
    case yoriito::viss::v1::ValueType::kValueObject: {
      return std::make_unique<yoriito::StructData>(
          ConvertValueObject(val.value_object()));
    }
    case yoriito::viss::v1::ValueType::kValueObjectArray:
      return ConvertValueObjectArray(val.value_object_array());
    default:
      break;
  }
  return ValueType();
}

}  // namespace yoriito::databroker
