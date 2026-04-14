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

#ifndef KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_INSPECT_UTIL_H_
#define KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_INSPECT_UTIL_H_

#include "databroker/base_inspect_data.h"
#include "yoriito/viss/v1/common.pb.h"
#include "result.h"

namespace yoriito::databroker {
class VissGrpcInspectUtil {
 public:
  static Result<BaseInspectData> ConvertInspectData(
      const yoriito::viss::v1::MetadataEntry& metadata);

  static yoriito::databroker::BaseInspectData::DataType ConvertDataType(
      yoriito::viss::v1::DataType data_type);
  static yoriito::ValueType ConvertValueData(yoriito::viss::v1::ValueType val);

  // EntryType
  static yoriito::viss::v1::NodeType ConvertEntryType(
      const yoriito::databroker::BaseInspectData::EntryType& entry_type);
  // DataTyoe
  static yoriito::viss::v1::DataType ConvertDataType(
      const yoriito::databroker::BaseInspectData::DataType& data_type);
  // Convert DataType to int32
  static Result<int32_t> ConvertDataTypeToInt(const ValueType& value);

 private:
  static yoriito::databroker::BaseInspectData::EntryType ConvertEntryType(
      const yoriito::viss::v1::MetadataEntry& metadata);

  static yoriito::ValueType ConvertInt32ToValueData(
      int32_t value, yoriito::databroker::BaseInspectData::DataType dataType);

  static yoriito::ValueType ConvertMinValueData(
      const yoriito::viss::v1::MetadataEntry& metadata);

  static yoriito::ValueType ConvertMaxValueData(
      const yoriito::viss::v1::MetadataEntry& metadata);
};
}  // namespace yoriito::databroker
#endif  // KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_INSPECT_UTIL_H_
