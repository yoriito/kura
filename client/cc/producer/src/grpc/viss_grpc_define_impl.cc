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

#include "grpc/viss_grpc_define_impl.h"

#include "common/inc/viss_common_error.h"
#include "grpc/viss_grpc_inspect_util.h"
#include "grpc/viss_grpc_value_util.h"
#include "yoriito/yoriito_log.h"

namespace yoriito::databroker::producer {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

void VissGrpcDefineImpl::CreateDefineRequest(
    yoriito::viss::v1::producer::DefineRequest &request,
    const ProducerDefineRequest &data) const {
  auto set_optional_value_type = [](yoriito::viss::v1::ValueType *dest,
                                    const yoriito::ValueType &src) {
    if (std::holds_alternative<std::monostate>(src)) {
      return;
    }
    const auto &result = VissGrpcValueUtil::SetDataPointValue(*dest, src);
    if (result.IsErr()) {
      YORIITO_LOG_ERROR("PRDR_DEFINE",
                        "VissGrpcValueUtil::SetDataPointValue failed");
    }
  };

  for (const auto &def : data.GetDefineEntries()) {
    auto *define = request.mutable_entries();
    auto *new_define = define->add_metadata_entries();
    new_define->set_path(def.GetPath());
    new_define->set_datatype(
        VissGrpcInspectUtil::ConvertDataType(def.GetDataType()));
    new_define->set_description(def.GetDescription());
    new_define->set_type(
        VissGrpcInspectUtil::ConvertEntryType(def.GetEntryType()));
    new_define->set_deprecated(def.GetDeprecation());
    new_define->set_unit(def.GetUnit());
    set_optional_value_type(new_define->mutable_min(), def.GetMin());
    set_optional_value_type(new_define->mutable_max(), def.GetMax());
    new_define->set_comment(def.GetComment());
    auto *allow = new_define->mutable_allowed();
    auto *val = allow->mutable_values();
    for (const auto &allowed : def.GetAllowed()) {
      *val->Add() = allowed;
    }
    // Set default value
    const auto &result = VissGrpcValueUtil::SetDataPointValue(
        *(new_define->mutable_default_()), def.GetDefaultValue());
    if (result.IsErr()) {
      YORIITO_LOG_ERROR("PRDR_DEFINE",
                        "VissGrpcValueUtil::SetDataPointValue failed");
    }
    new_define->set_arraysize(def.GetArraySize());
    auto *pattern = new_define->mutable_pattern();
    auto *pattern_val = pattern->mutable_values();
    for (const auto &allowed : def.GetPattern()) {
      *pattern_val->Add() = allowed;
    }

    for (const auto &[key, value] : def.GetCustomAttributes()) {
      auto custom = new_define->mutable_custom_metadata();
      (*custom->mutable_fields())[key].set_string_value(value);
    }
    if (def.GetDataTypeRef().has_value()) {
      new_define->set_datatype_path(def.GetDataTypeRef()->GetPath());
    }
  }
  for (const auto &struct_type : data.GetStructTypes()) {
    auto *define = request.mutable_entries();
    auto *new_define = define->add_struct_metadata_entries();
    new_define->set_path(struct_type.get()->GetPath());
    new_define->set_description(struct_type.get()->GetDescription());
    new_define->set_deprecated(struct_type.get()->GetDeprecation());
    new_define->set_comment(struct_type.get()->GetComment());

    const auto &custom_attributes = struct_type.get()->GetCustomAttributes();
    for (const auto &[key, value] : custom_attributes) {
      auto custom = new_define->mutable_custom_metadata();
      (*custom->mutable_fields())[key].set_string_value(value);
    }

    const auto &properties = struct_type.get()->GetProperties();
    for (const auto &property : properties) {
      auto *new_property = new_define->add_properties();
      new_property->set_name(property.GetPath());
      new_property->set_datatype(
          VissGrpcInspectUtil::ConvertDataType(property.GetDataType()));
      new_property->set_description(property.GetDescription());
      new_property->set_deprecated(property.GetDeprecation());
      new_property->set_unit(property.GetUnit());
      set_optional_value_type(new_property->mutable_min(), property.GetMin());
      set_optional_value_type(new_property->mutable_max(), property.GetMax());
      new_property->set_comment(property.GetComment());
      auto *allow = new_property->mutable_allowed();
      auto *val = allow->mutable_values();
      for (const auto &allowed : property.GetAllowed()) {
        *val->Add() = allowed;
      }
      new_property->set_arraysize(property.GetArraySize());
      for (const auto &[key, value] : property.GetCustomAttributes()) {
        auto custom = new_property->mutable_custom_metadata();
        (*custom->mutable_fields())[key].set_string_value(value);
      }
      if (property.GetDataTypeRef().IsResolved()) {
        new_property->set_datatype_path(property.GetDataTypeRef().GetPath());
      }
    }
  }
  return;
}

Result<void> VissGrpcDefineImpl::ConvertDefineResponse(
    grpc::Status status,
    const yoriito::viss::v1::producer::DefineResponse &response) const {
  if (status.ok()) {
    if (response.has_error()) {
      // VISS Error
      const std::string &error = response.error().description();
      const int32_t code = static_cast<int32_t>(response.error().number());
      const std::string &message = error + " Code:" + std::to_string(code);
      YORIITO_LOG_ERROR("PRDR_DEFINE", message);
      return Result<void>::Err(VissCommonError::CreateVissError(code, error));
    }

  } else {
    // GRPC Error
    auto error = status.error_message();
    YORIITO_LOG_ERROR("PRDR_DEFINE", error);
    return Result<void>::Err(VissCommonError::CreateProtocolError(error));
  }
  return Result<void>::Ok();
}

/**
 * @brief
 *
 * @param context
 * @param request
 * @param response
 * @return grpc::Status
 */
grpc::Status VissGrpcDefineImpl::Execute(
    ClientContext &context,
    const yoriito::viss::v1::producer::DefineRequest &request,
    yoriito::viss::v1::producer::DefineResponse &response) const {
  return stub_->Define(&context, request, &response);
}
}  // namespace yoriito::databroker::producer
