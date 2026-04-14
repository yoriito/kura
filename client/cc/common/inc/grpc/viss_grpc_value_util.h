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

#ifndef KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_VALUE_UTIL_H_
#define KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_VALUE_UTIL_H_

//
#include "databroker_type.h"
#include "yoriito/viss/v1/types/value.pb.h"
#include "result.h"

namespace yoriito::databroker {

class VissGrpcValueUtil final {
 public:
  static Result<void> SetDataPointValue(
      yoriito::viss::v1::ValueType &datapoint_value_type, ValueType value);
  static ValueType ConvertValueType(yoriito::viss::v1::ValueType val);
};

}  // namespace yoriito::databroker
#endif  // KURA_CLIENT_CC_COMMON_INC_GRPC_VISS_GRPC_VALUE_UTIL_H_
