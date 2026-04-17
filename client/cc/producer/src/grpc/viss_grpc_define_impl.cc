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

#include "grpc/viss_grpc_inspect_util.h"
#include "grpc/viss_grpc_value_util.h"
#include "viss_common_error.h"
#include "viss_common_log.h"
//
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.grpc.pb.h"
#include "yoriito/viss/v1/common.pb.h"
#include "yoriito/viss/v1/types/value.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;
namespace yoriito::databroker::producer {

void VissGrpcDefineImpl::CreateDefineRequest(
    yoriito::viss::v1::producer::DefineRequest& request,
    const ProducerDefineRequest& data) {
  for (const auto& def : data.GetDefineEntries()) {
    auto* define = request.mutable_entries();
    auto* new_define = define->add_entries();
    new_define->set_path(def.GetPath());
    new_define->set_datatype(
        VissGrpcInspectUtil::ConvertDataType(def.GetDataType()));
    new_define->set_description(def.GetDescription());
    new_define->set_type(
        VissGrpcInspectUtil::ConvertEntryType(def.GetEntryType()));
    new_define->set_deprecated(def.GetDeprecation());
    new_define->set_unit(def.GetUnit());
    auto min = VissGrpcInspectUtil::ConvertDataTypeToInt(def.GetMin());
    auto max = VissGrpcInspectUtil::ConvertDataTypeToInt(def.GetMax());
    if (min.IsOk()) {
      new_define->set_min(min.TakeOk().value());
    }
    if (max.IsOk()) {
      new_define->set_max(max.TakeOk().value());
    }
    new_define->set_comment(def.GetComment());
    auto* allow = new_define->mutable_allowed();
    auto* val = allow->mutable_values();
    for (const auto& allowed : def.GetAllowed()) {
      *val->Add() = allowed;
    }
    // Set default value
    VissGrpcValueUtil::SetDataPointValue(*(new_define->mutable_default_()),
                                         def.GetDefaultValue());
    new_define->set_arraysize(def.GetArraySize());
    auto* pattern = new_define->mutable_pattern();
    auto* pattern_val = pattern->mutable_values();
    for (const auto& allowed : def.GetPattern()) {
      *pattern_val->Add() = allowed;
    }

    for (const auto& [key, value] : def.GetCustomAttributes()) {
      auto custom = new_define->mutable_custom_metadata();
      (*custom->mutable_fields())[key].set_string_value(value);
    }
  }
  return;
}

Result<void> VissGrpcDefineImpl::ConvertDefineResponse(
    grpc::Status status, yoriito::viss::v1::producer::DefineResponse& response) {
  if (status.ok()) {
    if (response.has_error()) {
      // VISS Error
      auto error = response.error().description();
      auto code = response.error().number();
      auto message = error + " Code:" + std::to_string(code);
      VISS_LOG_ERROR("PRDR_DEFINE", message);
      return Result<void>::Err(VissCommonError::CreateVissError(code, error));
    }

  } else {
    // GRPC Error
    auto error = status.error_message();
    VISS_LOG_ERROR("PRDR_DEFINE", error);
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
    ClientContext& context, yoriito::viss::v1::producer::DefineRequest& request,
    yoriito::viss::v1::producer::DefineResponse& response) {
  return stub_->Define(&context, request, &response);
}
}  // namespace yoriito::databroker::producer
