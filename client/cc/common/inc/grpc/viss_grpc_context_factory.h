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

#ifndef KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_CONTEXT_FACTORY_H_
#define KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_CONTEXT_FACTORY_H_

#include <memory>

#include "grpcpp/grpcpp.h"

namespace yoriito::databroker {

class ClientContextFactory {
 public:
  struct Config {
    int timeout_seconds = 3;
  };
  ClientContextFactory() = delete;
  static std::unique_ptr<grpc::ClientContext> Create(const Config& config) {
    auto context = std::make_unique<grpc::ClientContext>();
    if (config.timeout_seconds > 0) {
      context->set_deadline(std::chrono::system_clock::now() +
                            std::chrono::seconds(config.timeout_seconds));
    }
    return context;
  }
};

}  // namespace yoriito::databroker

#endif  // KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_CONTEXT_FACTORY_H_
