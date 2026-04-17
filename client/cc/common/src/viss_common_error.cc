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

#include "viss_common_error.h"

namespace yoriito::databroker {

std::unique_ptr<yoriito::CommonError> VissCommonError::CreateVissError(
    int32_t code, const std::string& msg) {
  return std::make_unique<yoriito::CommonError>(
      static_cast<yoriito::ErrorCode>(code), msg);
}

std::unique_ptr<yoriito::CommonError> VissCommonError::CreateCancelError(
    const std::string& msg) {
  return std::make_unique<yoriito::CommonError>(
      yoriito::ErrorCode::ERROR_CANCELED, msg);
}

std::unique_ptr<yoriito::CommonError> VissCommonError::CreateProtocolError(
    const std::string& msg) {
  return std::make_unique<yoriito::CommonError>(
      yoriito::ErrorCode::ERROR_CONNREFUSED, msg);
}
std::unique_ptr<yoriito::CommonError> VissCommonError::CreateIllegalError(
    const std::string& msg) {
  return std::make_unique<yoriito::CommonError>(yoriito::ErrorCode::ERROR_INVAL,
                                               msg);
}

std::unique_ptr<yoriito::CommonError> VissCommonError::CreateNotFoundError(
    const std::string& msg) {
  return std::make_unique<yoriito::CommonError>(
      yoriito::ErrorCode::ERROR_NOT_FOUND, msg);
}

}  // namespace yoriito::databroker
