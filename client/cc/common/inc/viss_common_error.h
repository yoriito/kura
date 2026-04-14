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

#ifndef KURA_CLIENT_CC_COMMON_INC_VISS_COMMON_ERROR_H_
#define KURA_CLIENT_CC_COMMON_INC_VISS_COMMON_ERROR_H_

#include <memory>
#include <string>

#include "error.h"

namespace yoriito::databroker {
class VissCommonError {
 public:
  static std::unique_ptr<yoriito::CommonError> CreateVissError(
      int32_t code, const std::string& msg);
  static std::unique_ptr<yoriito::CommonError> CreateCancelError(
      const std::string& msg);
  static std::unique_ptr<yoriito::CommonError> CreateNotFoundError(
      const std::string& msg);
  static std::unique_ptr<yoriito::CommonError> CreateIllegalError(
      const std::string& msg);
  static std::unique_ptr<yoriito::CommonError> CreateProtocolError(
      const std::string& msg);
};
}  // namespace yoriito::databroker
#endif  // KURA_CLIENT_CC_COMMON_INC_VISS_COMMON_ERROR_H_
