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

#ifndef COMMON_INC_GRPC_GRPC_INTERCEPTOR_FACTORY_H_
#define COMMON_INC_GRPC_GRPC_INTERCEPTOR_FACTORY_H_

#include <grpcpp/support/client_interceptor.h>

#include "grpc/viss_grpc_interceptor.h"

namespace yoriito::databroker {

class GrpcInterceptorFactory
    : public grpc::experimental::ClientInterceptorFactoryInterface {
 public:
  explicit GrpcInterceptorFactory(DataBrokerType databrokerType)
      : grpc::experimental::ClientInterceptorFactoryInterface(),
        databrokerType_(databrokerType) {}

  grpc::experimental::Interceptor* CreateClientInterceptor(
      grpc::experimental::ClientRpcInfo* info) override {
    auto interceptor =
        std::make_unique<VissGrpcInterceptor>(databrokerType_, *info);
    return interceptor.release();
  }

 private:
  DataBrokerType databrokerType_;
};
}  // namespace yoriito::databroker
#endif  // COMMON_INC_GRPC_GRPC_INTERCEPTOR_FACTORY_H_
