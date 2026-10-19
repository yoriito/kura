/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright 2026 JASPAR
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INC_GRPC_VISS_GRPC_VALUE_UTIL_H_
#define COMMON_INC_GRPC_VISS_GRPC_VALUE_UTIL_H_

#include "yoriito/viss/v1/types/value.pb.h"
//
#include "yoriito/databroker_type.h"
#include "yoriito/result.h"

namespace yoriito::databroker {

class VissGrpcValueUtil final {
 public:
  static Result<void> SetDataPointValue(
      yoriito::viss::v1::ValueType &datapoint_value_type,
      const ValueType &value);
  static ValueType ConvertValueType(yoriito::viss::v1::ValueType val);

 private:
  static Result<void> SetStructDataValue(
      yoriito::viss::v1::ValueType &datapoint_value_type,
      const StructDataPtr &value);
  static Result<void> SetStructDataArrayValue(
      yoriito::viss::v1::ValueType &datapoint_value_type,
      const StructDataArray &value);
  static void SetValueObject(yoriito::viss::v1::ValueObject &value_object,
                             const StructData &struct_data);
  static StructData ConvertValueObject(
      const yoriito::viss::v1::ValueObject &value_object);
  static ValueType ConvertValueObjectArray(
      const yoriito::viss::v1::ValueObjectArray &value_object_array);

  template <typename T, typename Array>
  static ValueType ConvertArray(const Array &array);

  template <typename T, typename MutableArray>
  static void SetArrayValue(yoriito::viss::v1::ValueType &datapoint_value_type,
                            const ValueType &value, MutableArray mutable_array);
};

}  // namespace yoriito::databroker
#endif  // COMMON_INC_GRPC_VISS_GRPC_VALUE_UTIL_H_
