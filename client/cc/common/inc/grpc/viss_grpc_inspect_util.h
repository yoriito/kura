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

#ifndef COMMON_INC_GRPC_VISS_GRPC_INSPECT_UTIL_H_
#define COMMON_INC_GRPC_VISS_GRPC_INSPECT_UTIL_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "yoriito/viss/v1/common.pb.h"
//
#include "databroker/base_inspect_data.h"
#include "databroker/types/property_type.h"
#include "databroker/types/struct_type.h"
#include "yoriito/result.h"

namespace yoriito::databroker {
class VissGrpcInspectUtil {
 public:
  using StructTypeMap =
      std::map<std::string, std::shared_ptr<yoriito::databroker::StructType>>;

  static Result<BaseInspectData> ConvertInspectData(
      const yoriito::viss::v1::MetadataEntry &metadata);

  static yoriito::databroker::BaseInspectData::DataType ConvertDataType(
      yoriito::viss::v1::DataType data_type);

  // EntryType
  static yoriito::viss::v1::NodeType ConvertEntryType(
      const yoriito::databroker::BaseInspectData::EntryType &entry_type);
  // DataTyoe
  static yoriito::viss::v1::DataType ConvertDataType(
      const yoriito::databroker::BaseInspectData::DataType &data_type);
  //
  static Result<yoriito::databroker::StructType> ConvertStructData(
      const yoriito::viss::v1::StructMetadataEntry &metadata);

  static Result<yoriito::databroker::PropertyType> ConvertPropertyData(
      const yoriito::viss::v1::PropertyMetadataEntry &metadata);

  static void BuildAndResolveStructTypeMap(
      const yoriito::viss::v1::InspectResponse &response,
      StructTypeMap &struct_type_map);

  static bool IsStructDataType(
      yoriito::databroker::BaseInspectData::DataType data_type);

 private:
  static yoriito::databroker::BaseInspectData::EntryType ConvertEntryType(
      const yoriito::viss::v1::MetadataEntry &metadata);

  static yoriito::ValueType ConvertMinValueData(
      const yoriito::viss::v1::MetadataEntry &metadata);

  static yoriito::ValueType ConvertMaxValueData(
      const yoriito::viss::v1::MetadataEntry &metadata);

  static std::map<std::string, std::string> ConvertCustomMetadata(
      const google::protobuf::Struct &custom_metadata);
};
}  // namespace yoriito::databroker
#endif  // COMMON_INC_GRPC_VISS_GRPC_INSPECT_UTIL_H_
