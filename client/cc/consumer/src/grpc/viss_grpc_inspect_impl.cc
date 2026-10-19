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

#include "grpc/viss_grpc_inspect_impl.h"
//
#include <string>
#include <utility>
#include <vector>
//
#include "common/inc/viss_common_error.h"
#include "grpc/viss_grpc_inspect_util.h"
#include "grpc/viss_grpc_value_util.h"
#include "yoriito/yoriito_log.h"
//

namespace yoriito::databroker::consumer {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using yoriito::viss::v1::consumer::VISS;

void VissGrpcInspectImpl::CreateInspectRequest(
    yoriito::viss::v1::InspectRequest &request,
    const ConsumerInspectRequest &data) const {
  request.set_path(data.GetPath());
  auto get_filter = request.mutable_filter();
  auto path_filter = get_filter->mutable_paths_parameter();
  if (data.GetFilter().has_value()) {
    auto paths = data.GetFilter().value().GetPaths();
    for (const std::string &path : paths) {
      path_filter->add_paths(path);
    }
  }
  return;
}

grpc::Status VissGrpcInspectImpl::Execute(
    ClientContext &context, const yoriito::viss::v1::InspectRequest &request,
    yoriito::viss::v1::InspectResponse &response) const {
  return stub_->Inspect(&context, request, &response);
}

Result<ConsumerInspectResponse> VissGrpcInspectImpl::ConvertInspectResponse(
    grpc::Status status, const yoriito::viss::v1::InspectResponse &response,
    const ConsumerInspectRequest &data) const {
  std::vector<yoriito::databroker::ConsumerInspectData> inspect_data_list;
  std::map<std::string, std::shared_ptr<yoriito::databroker::StructType>>
      struct_type_map;

  if (status.ok()) {
    if (response.has_error()) {
      // VISS Error
      const std::string &error = response.error().description();
      const int32_t code = static_cast<int32_t>(response.error().number());
      const std::string &message = error + " Code:" + std::to_string(code);
      YORIITO_LOG_ERROR("CNSR_INSPECT", message);
      return Result<ConsumerInspectResponse>::Err(
          VissCommonError::CreateVissError(code, error));
    }

    // Illegal Response: No Data( actually return viss error )
    if (!response.has_metadata() ||
        response.metadata().metadata_entries().empty()) {
      YORIITO_LOG_ERROR(
          "CNSR_INSPECT",
          "Response OK but response consistency failed." + data.GetPath());
      return Result<ConsumerInspectResponse>::Err(
          VissCommonError::CreateIllegalError(
              "Response OK but response consistency failed." + data.GetPath()));
    }

    VissGrpcInspectUtil::BuildAndResolveStructTypeMap(response,
                                                      struct_type_map);
    BuildInspectDataList(response, struct_type_map, inspect_data_list);

  } else {
    // GRPC Error
    auto error = status.error_message();
    YORIITO_LOG_ERROR("CNSR_INSPECT", error);
    return Result<ConsumerInspectResponse>::Err(
        VissCommonError::CreateProtocolError(error));
  }
  std::vector<std::shared_ptr<yoriito::databroker::StructType>>
      struct_type_list;
  struct_type_list.reserve(struct_type_map.size());

  (void)std::transform(struct_type_map.begin(), struct_type_map.end(),
                       std::back_inserter(struct_type_list),
                       [](const auto &pair) { return pair.second; });
  yoriito::databroker::ConsumerInspectResponse inspect_response(
      std::move(inspect_data_list), std::move(struct_type_list));
  return Result<ConsumerInspectResponse>::Ok(std::move(inspect_response));
}

void VissGrpcInspectImpl::BuildInspectDataList(
    const yoriito::viss::v1::InspectResponse &response,
    const std::map<std::string,
                   std::shared_ptr<yoriito::databroker::StructType>>
        &struct_type_map,
    std::vector<yoriito::databroker::ConsumerInspectData> &inspect_data_list)
    const {
  const auto &metadata_entries = response.metadata().metadata_entries();
  for (const auto &metadata_package : metadata_entries) {
    auto ret = VissGrpcInspectUtil::ConvertInspectData(metadata_package);
    if (!ret.IsOk()) {
      continue;
    }

    auto value = ret.TakeOk().value();
    if (VissGrpcInspectUtil::IsStructDataType(value.GetDataType())) {
      const std::string data_type_ref = metadata_package.datatype_path();
      if (!data_type_ref.empty()) {
        auto it = struct_type_map.find(data_type_ref);
        if (it != struct_type_map.end()) {
          value.SetDataTypeRef(yoriito::databroker::StructTypeRef(it->second));
        }
      }
    }
    inspect_data_list.push_back(
        yoriito::databroker::ConsumerInspectData(value));
  }
}

}  // namespace yoriito::databroker::consumer
