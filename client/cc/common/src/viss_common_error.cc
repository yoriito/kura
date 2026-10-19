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

#include "common/inc/viss_common_error.h"

#include "yoriito/viss/v1/types/error.pb.h"

namespace yoriito::databroker {

yoriito::ErrorCode VissCommonError::ConvertVissErrorCode(int32_t code) {
  switch (code) {
    case yoriito::viss::v1::ERROR_UNSPECIFIED:
      return yoriito::ErrorCode::ERROR_UNSPECIFIED;
    case yoriito::viss::v1::ERROR_BAD_REQUEST:
      return yoriito::ErrorCode::ERROR_BAD_REQUEST;
    case yoriito::viss::v1::ERROR_UNAUTHORIZED:
      return yoriito::ErrorCode::ERROR_UNAUTHORIZED;
    case yoriito::viss::v1::ERROR_FORBIDDEN:
      return yoriito::ErrorCode::ERROR_FORBIDDEN;
    case yoriito::viss::v1::ERROR_NOT_FOUND:
      return yoriito::ErrorCode::ERROR_NOT_FOUND;
    case yoriito::viss::v1::ERROR_REQUEST_TIMEOUT:
      return yoriito::ErrorCode::ERROR_REQUEST_TIMEOUT;
    case yoriito::viss::v1::ERROR_TOO_MANY_REQUESTS:
      return yoriito::ErrorCode::ERROR_TOO_MANY_REQUESTS;
    case yoriito::viss::v1::ERROR_INTERNAL_SERVER_ERROR:
      return yoriito::ErrorCode::ERROR_UNKNOWN;
    case yoriito::viss::v1::ERROR_BAD_GATEWAY:
      return yoriito::ErrorCode::ERROR_BAD_GATEWAY;
    case yoriito::viss::v1::ERROR_SERVICE_UNAVAILABLE:
      return yoriito::ErrorCode::ERROR_SERVICE_UNAVAILABLE;
    case yoriito::viss::v1::ERROR_GATEWAY_TIMEOUT:
      return yoriito::ErrorCode::ERROR_GATEWAY_TIMEOUT;
    default:
      return yoriito::ErrorCode::ERROR_UNKNOWN;
  }
}

std::unique_ptr<yoriito::CommonError> VissCommonError::CreateVissError(
    int32_t code, const std::string& msg) {
  return std::make_unique<yoriito::CommonError>(ConvertVissErrorCode(code),
                                                msg);
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
