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
#include <vector>

#include "viss_common_error.h"

using yoriito::viss::v1::BoolArray;
using yoriito::viss::v1::DoubleArray;
using yoriito::viss::v1::FloatArray;
using yoriito::viss::v1::Int16Array;
using yoriito::viss::v1::Int32Array;
using yoriito::viss::v1::Int64Array;
using yoriito::viss::v1::Int8Array;
using yoriito::viss::v1::StringArray;
using yoriito::viss::v1::Uint16Array;
using yoriito::viss::v1::Uint32Array;
using yoriito::viss::v1::Uint64Array;
using yoriito::viss::v1::Uint8Array;

namespace yoriito::databroker {

Result<void> VissGrpcValueUtil::SetDataPointValue(
    yoriito::viss::v1::ValueType &datapoint_value_type, ValueType value) {
  if (std::holds_alternative<std::string>(value)) {
    // std::string,
    std::string val = std::get<std::string>(value);
    datapoint_value_type.set_string(val);
  } else if (std::holds_alternative<bool>(value)) {
    // bool,
    bool val = std::get<bool>(value);
    datapoint_value_type.set_bool_(val);
  } else if (std::holds_alternative<int8_t>(value)) {
    // int8_t,
    int8_t val = std::get<int8_t>(value);
    datapoint_value_type.set_int8(val);
  } else if (std::holds_alternative<int16_t>(value)) {
    // int16_t,
    int16_t val = std::get<int16_t>(value);
    datapoint_value_type.set_int16(val);
  } else if (std::holds_alternative<int32_t>(value)) {
    // int32_t,
    int32_t val = std::get<int32_t>(value);
    datapoint_value_type.set_int32(val);
  } else if (std::holds_alternative<int64_t>(value)) {
    // int64_t,
    int64_t val = std::get<int64_t>(value);
    datapoint_value_type.set_int64(val);
  } else if (std::holds_alternative<uint8_t>(value)) {
    // uint8_t,
    uint8_t val = std::get<uint8_t>(value);
    datapoint_value_type.set_uint8(val);
  } else if (std::holds_alternative<uint16_t>(value)) {
    // uint16_t,
    uint16_t val = std::get<uint16_t>(value);
    datapoint_value_type.set_uint16(val);
  } else if (std::holds_alternative<uint32_t>(value)) {
    // uint32_t,
    uint32_t val = std::get<uint32_t>(value);
    datapoint_value_type.set_uint32(val);
  } else if (std::holds_alternative<uint64_t>(value)) {
    // uint64_t,
    uint64_t val = std::get<uint64_t>(value);
    datapoint_value_type.set_uint64(val);
  } else if (std::holds_alternative<float>(value)) {
    // float,
    float val = std::get<float>(value);
    datapoint_value_type.set_float_(val);
  } else if (std::holds_alternative<double>(value)) {
    // double,
    double val = std::get<double>(value);
    datapoint_value_type.set_double_(val);
  } else if (std::holds_alternative<std::vector<std::string>>(value)) {
    // std::vector<std::string>,
    std::vector<std::string> val = std::get<std::vector<std::string>>(value);
    auto *stringArray = datapoint_value_type.mutable_string_array();
    for (const auto &elem : val) {
      stringArray->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<bool>>(value)) {
    // std::vector<bool>,
    std::vector<bool> val = std::get<std::vector<bool>>(value);
    auto *boolArray = datapoint_value_type.mutable_bool_array();
    for (const auto &elem : val) {
      boolArray->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<int8_t>>(value)) {
    // std::vector<int8_t>,
    std::vector<int8_t> val = std::get<std::vector<int8_t>>(value);
    auto *int8Array = datapoint_value_type.mutable_int8_array();
    for (const auto &elem : val) {
      int8Array->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<int16_t>>(value)) {
    // std::vector<int16_t>,
    std::vector<int16_t> val = std::get<std::vector<int16_t>>(value);
    auto *int16Array = datapoint_value_type.mutable_int16_array();
    for (const auto &elem : val) {
      int16Array->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<int32_t>>(value)) {
    // std::vector<int32_t>
    std::vector<int32_t> val = std::get<std::vector<int32_t>>(value);
    auto *int32Array = datapoint_value_type.mutable_int32_array();
    for (const auto &elem : val) {
      int32Array->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<int64_t>>(value)) {
    // std::vector<int64_t>,
    std::vector<int64_t> val = std::get<std::vector<int64_t>>(value);
    auto *int64Array = datapoint_value_type.mutable_int64_array();
    for (const auto &elem : val) {
      int64Array->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<uint8_t>>(value)) {
    // std::vector<uint8_t>,
    std::vector<uint8_t> val = std::get<std::vector<uint8_t>>(value);
    auto *uint8Array = datapoint_value_type.mutable_uint8_array();
    for (const auto &elem : val) {
      uint8Array->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<uint16_t>>(value)) {
    // std::vector<uint16_t>,
    std::vector<uint16_t> val = std::get<std::vector<uint16_t>>(value);
    auto *uint16Array = datapoint_value_type.mutable_uint16_array();
    for (const auto &elem : val) {
      uint16Array->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<uint32_t>>(value)) {
    // std::vector<uint32_t>,
    std::vector<uint32_t> val = std::get<std::vector<uint32_t>>(value);
    auto *uint32Array = datapoint_value_type.mutable_uint32_array();
    for (const auto &elem : val) {
      uint32Array->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<uint64_t>>(value)) {
    // std::vector<uint64_t>,
    std::vector<uint64_t> val = std::get<std::vector<uint64_t>>(value);
    auto *uint64Array = datapoint_value_type.mutable_uint64_array();
    for (const auto &elem : val) {
      uint64Array->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<float>>(value)) {
    // std::vector<float>,
    std::vector<float> val = std::get<std::vector<float>>(value);
    auto *floatArray = datapoint_value_type.mutable_float_array();
    for (const auto &elem : val) {
      floatArray->add_values(elem);
    }
  } else if (std::holds_alternative<std::vector<double>>(value)) {
    // std::vector<double>
    std::vector<double> val = std::get<std::vector<double>>(value);
    auto *doubleArray = datapoint_value_type.mutable_double_array();
    for (const auto &elem : val) {
      doubleArray->add_values(elem);
    }
  } else {
    return Result<void>::Err(

        VissCommonError::CreateIllegalError("value type error"));
  }
  return Result<void>::Ok();
}

ValueType VissGrpcValueUtil::ConvertValueType(yoriito::viss::v1::ValueType val) {
  ValueType ret;
  switch (val.value_type_case()) {
    case yoriito::viss::v1::ValueType::kUint8:
      ret = ValueType(static_cast<uint8_t>(val.uint8()));
      break;
    case yoriito::viss::v1::ValueType::kUint16:
      ret = ValueType(static_cast<uint16_t>(val.uint16()));
      break;
    case yoriito::viss::v1::ValueType::kUint32:
      ret = ValueType(static_cast<uint32_t>(val.uint32()));
      break;
    case yoriito::viss::v1::ValueType::kUint64:
      ret = ValueType(static_cast<uint64_t>(val.uint64()));
      break;
    case yoriito::viss::v1::ValueType::kInt8:
      ret = ValueType(static_cast<int8_t>(val.int8()));
      break;
    case yoriito::viss::v1::ValueType::kInt16:
      ret = ValueType(static_cast<int16_t>(val.int16()));
      break;
    case yoriito::viss::v1::ValueType::kInt32:
      ret = ValueType(static_cast<int32_t>(val.int32()));
      break;
    case yoriito::viss::v1::ValueType::kInt64:
      ret = ValueType(static_cast<int64_t>(val.int64()));
      break;
    case yoriito::viss::v1::ValueType::kFloat:
      ret = ValueType(val.float_());
      break;
    case yoriito::viss::v1::ValueType::kDouble:
      ret = ValueType(val.double_());
      break;
    case yoriito::viss::v1::ValueType::kString:
      ret = ValueType(val.string());
      break;
    case yoriito::viss::v1::ValueType::kBool:
      ret = ValueType(val.bool_());
      break;
    case yoriito::viss::v1::ValueType::kUint8Array: {
      const Uint8Array &uint8Array = val.uint8_array();
      std::vector<uint8_t> retValue;
      for (int i = 0; i < uint8Array.values_size(); ++i) {
        retValue.push_back(static_cast<uint8_t>(uint8Array.values(i)));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kUint16Array: {
      const Uint16Array &uint16Array = val.uint16_array();
      std::vector<uint16_t> retValue;
      for (int i = 0; i < uint16Array.values_size(); ++i) {
        retValue.push_back(static_cast<uint16_t>(uint16Array.values(i)));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kUint32Array: {
      const Uint32Array &uint32Array = val.uint32_array();
      std::vector<uint32_t> retValue;
      for (int i = 0; i < uint32Array.values_size(); ++i) {
        retValue.push_back(static_cast<uint32_t>(uint32Array.values(i)));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kUint64Array: {
      const Uint64Array &uint64Array = val.uint64_array();
      std::vector<uint64_t> retValue;
      for (int i = 0; i < uint64Array.values_size(); ++i) {
        retValue.push_back(static_cast<uint64_t>(uint64Array.values(i)));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kInt8Array: {
      const Int8Array &int8Array = val.int8_array();
      std::vector<int8_t> retValue;
      for (int i = 0; i < int8Array.values_size(); ++i) {
        retValue.push_back(static_cast<int8_t>(int8Array.values(i)));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kInt16Array: {
      const Int16Array &int16Array = val.int16_array();
      std::vector<int16_t> retValue;
      for (int i = 0; i < int16Array.values_size(); ++i) {
        retValue.push_back(static_cast<int16_t>(int16Array.values(i)));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kInt32Array: {
      const Int32Array &int32Array = val.int32_array();
      std::vector<int32_t> retValue;
      for (int i = 0; i < int32Array.values_size(); ++i) {
        retValue.push_back(static_cast<int32_t>(int32Array.values(i)));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kInt64Array: {
      const Int64Array &int64Array = val.int64_array();
      std::vector<int64_t> retValue;
      for (int i = 0; i < int64Array.values_size(); ++i) {
        retValue.push_back(static_cast<int64_t>(int64Array.values(i)));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kFloatArray: {
      const FloatArray &floatArray = val.float_array();
      std::vector<float> retValue;
      for (int i = 0; i < floatArray.values_size(); ++i) {
        retValue.push_back(floatArray.values(i));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kDoubleArray: {
      const DoubleArray &doubleArray = val.double_array();
      std::vector<double> retValue;
      for (int i = 0; i < doubleArray.values_size(); ++i) {
        retValue.push_back(doubleArray.values(i));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kStringArray: {
      const StringArray &stringArray = val.string_array();
      std::vector<std::string> retValue;
      for (int i = 0; i < stringArray.values_size(); ++i) {
        retValue.push_back(stringArray.values(i));
      }
      ret = ValueType(retValue);
      break;
    }
    case yoriito::viss::v1::ValueType::kBoolArray: {
      const BoolArray &boolArray = val.bool_array();
      std::vector<bool> retValue;
      for (int i = 0; i < boolArray.values_size(); ++i) {
        retValue.push_back(boolArray.values(i));
      }
      ret = ValueType(retValue);
      break;
    }
    default:
      break;
  }
  return ret;
}

}  // namespace yoriito::databroker
