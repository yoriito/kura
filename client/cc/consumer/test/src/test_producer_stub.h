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

#ifndef KURA_CLIENT_CC_CONSUMER_TEST_SRC_TEST_PRODUCER_STUB_H_
#define KURA_CLIENT_CC_CONSUMER_TEST_SRC_TEST_PRODUCER_STUB_H_

#include <string>

#include "yoriito/databroker_type.h"
#include "yoriito/result.h"

namespace yoriito::databroker::test_stub {
class ProducerStub {
 public:
  Result<void> SetData(const std::string &path,
                       const yoriito::ValueType &value);
  Result<void> SetData(const std::string &path, const yoriito::ValueType &value,
                       const bool is_available_sensor,
                       const bool is_available_actuator);
  Result<void> SetData(const std::string &path, const yoriito::ValueType &value,
                       const bool is_available_sensor,
                       const bool is_available_actuator, int64_t timestamp);
  Result<ValueType> GetData(const std::string &path);
};
}  // namespace yoriito::databroker::test_stub
#endif  // KURA_CLIENT_CC_CONSUMER_TEST_SRC_TEST_PRODUCER_STUB_H_
