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

#ifndef KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_INTERCEPTOR_H_
#define KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_INTERCEPTOR_H_

#include <google/protobuf/message.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/client_interceptor.h>

#include <string>

#include "viss_common_log.h"

namespace yoriito::databroker {
enum class DataBrokerType { kConsumer, kProducer };
class VissGrpcInterceptor : public grpc::experimental::Interceptor {
 public:
  explicit VissGrpcInterceptor(const DataBrokerType& databrokerType,
                               const grpc::experimental::ClientRpcInfo& info) {
    if (databrokerType == DataBrokerType::kConsumer) {
      label_ = "CNSR_GRPC";
    } else {
      label_ = "PRDR_GRPC";
    }
    full_method_ = info.method();
  }
  void Intercept(
      grpc::experimental::InterceptorBatchMethods* methods) override {
    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::PRE_SEND_MESSAGE)) {
      const auto* msg_ptr = methods->GetSendMessage();
      const auto* message =
          static_cast<const google::protobuf::Message*>(msg_ptr);
      if (message) {
        VISS_LOG_DEBUG(label_, "[gRPC] Request:" + full_method_ +
                                   " data:" + message->ShortDebugString());
      }
    }

    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::POST_RECV_MESSAGE)) {
      const void* msg_ptr = methods->GetRecvMessage();
      if (msg_ptr) {
        const auto* message =
            static_cast<const google::protobuf::Message*>(msg_ptr);
        VISS_LOG_DEBUG(label_, "[gRPC] Response:" + full_method_ +
                                   " data:" + message->ShortDebugString());
      }
    }
    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::POST_RECV_STATUS)) {
      auto* status = methods->GetRecvStatus();
      if (!status->ok()) {
        VISS_LOG_ERROR(label_, "[gRPC] Error: " + status->error_message());
      }
    }

    methods->Proceed();
  }

 private:
  std::string label_;
  std::string full_method_;
};
}  // namespace yoriito::databroker
#endif  // KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_INTERCEPTOR_H_
