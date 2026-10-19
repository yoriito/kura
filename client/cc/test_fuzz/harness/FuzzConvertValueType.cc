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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <variant>

#ifndef __AFL_INIT
#define __AFL_INIT() \
  do {               \
  } while (0)
#define __AFL_LOOP(x)         \
  ([] {                       \
    static bool first = true; \
    if (first) {              \
      first = false;          \
      return true;            \
    }                         \
    return false;             \
  }())
#endif

#include "grpc/viss_grpc_value_util.h"
#include "yoriito/viss/v1/types/value.pb.h"

int main(int argc, char* argv[]) {
  __AFL_INIT();

  uint8_t buf[4096];

  while (__AFL_LOOP(1000)) {
    size_t len = fread(buf, 1, sizeof(buf), stdin);
    if (len == 0) continue;

    yoriito::viss::v1::ValueType proto_value;
    // Ignore parse errors; an unset proto exercises the default branch.
    proto_value.ParseFromArray(buf, static_cast<int>(len));

    // ConvertValueType returns yoriito::ValueType (=
    // yoriito::databroker::ValueType)
    yoriito::ValueType result =
        yoriito::databroker::VissGrpcValueUtil::ConvertValueType(proto_value);

    // Visit every variant to ensure no crash occurs.
    std::visit([](auto&&) {}, result);

    // Verify that the oneof field and the converted variant are consistent.
    using VT = yoriito::viss::v1::ValueType;
    switch (proto_value.value_type_case()) {
      case VT::kUint8:
        assert(std::holds_alternative<uint8_t>(result));
        break;
      case VT::kUint16:
        assert(std::holds_alternative<uint16_t>(result));
        break;
      case VT::kFloat:
        assert(std::holds_alternative<float>(result));
        break;
      case VT::kString:
        assert(std::holds_alternative<std::string>(result));
        break;
      case VT::VALUE_TYPE_NOT_SET:
        assert(std::holds_alternative<std::monostate>(result));
        break;
      default:
        break;
    }
  }

  return 0;
}
