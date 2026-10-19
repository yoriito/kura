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

#ifndef CONSUMER_INC_GRPC_VISS_GRPC_INSPECT_IMPL_H_
#define CONSUMER_INC_GRPC_VISS_GRPC_INSPECT_IMPL_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "databroker/consumer_inspect_request.h"
#include "databroker/consumer_inspect_response.h"
#include "yoriito/result.h"
//
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/common.pb.h"
#include "yoriito/viss/v1/consumer.grpc.pb.h"

namespace yoriito::databroker::consumer {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using yoriito::viss::v1::consumer::VISS;

class VissGrpcInspectImpl {
 public:
  explicit VissGrpcInspectImpl(yoriito::viss::v1::consumer::VISS::Stub* channel)
      : stub_(channel) {}
  ~VissGrpcInspectImpl() = default;

  void CreateInspectRequest(yoriito::viss::v1::InspectRequest& request,
                            const ConsumerInspectRequest& data) const;
  grpc::Status Execute(ClientContext& context,
                       const yoriito::viss::v1::InspectRequest& request,
                       yoriito::viss::v1::InspectResponse& response) const;
  Result<ConsumerInspectResponse> ConvertInspectResponse(
      grpc::Status status, const yoriito::viss::v1::InspectResponse& response,
      const ConsumerInspectRequest& data) const;

 private:
  void BuildInspectDataList(
      const yoriito::viss::v1::InspectResponse& response,
      const std::map<std::string,
                     std::shared_ptr<yoriito::databroker::StructType>>&
          struct_type_map,
      std::vector<yoriito::databroker::ConsumerInspectData>& inspect_data_list)
      const;

  yoriito::viss::v1::consumer::VISS::Stub* stub_;
};
}  // namespace yoriito::databroker::consumer
#endif  // CONSUMER_INC_GRPC_VISS_GRPC_INSPECT_IMPL_H_
