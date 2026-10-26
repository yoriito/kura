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
#include <string>

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

#include "databroker/consumer_get_request.h"
#include "grpc/viss_grpc_get_impl.h"
#include "yoriito/viss/v1/consumer.pb.h"

int main(int argc, char* argv[]) {
  __AFL_INIT();

  uint8_t buf[65536];

  while (__AFL_LOOP(1000)) {
    size_t len = fread(buf, 1, sizeof(buf), stdin);
    if (len == 0) continue;

    yoriito::viss::v1::consumer::GetCurrentResponse response;
    response.ParseFromArray(buf, static_cast<int>(len));

    yoriito::databroker::ConsumerGetRequest dummy_request(
        std::string("Vehicle.Speed"));

    grpc::Status ok_status = grpc::Status::OK;

    yoriito::databroker::consumer::VissGrpcGetImpl impl(nullptr);
    auto result = impl.ConvertGetResponse(ok_status, response, dummy_request);

    if (result.IsErr()) {
      assert(result.TakeErr() != nullptr);
    } else {
      auto resp = result.TakeOk();
      assert(resp.has_value());
    }
  }

  return 0;
}
