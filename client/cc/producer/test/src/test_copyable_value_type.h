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

#ifndef KURA_CLIENT_CC_COMMON_INC_TEST_COPYABLE_VALUE_TYPE_H_
#define KURA_CLIENT_CC_COMMON_INC_TEST_COPYABLE_VALUE_TYPE_H_

#include <type_traits>
#include <utility>

#include "yoriito/databroker_type.h"

namespace yoriito::test {

struct CopyableValueType {
  CopyableValueType() = default;

  explicit CopyableValueType(yoriito::ValueType v) : value(std::move(v)) {}

  template <typename T, typename = std::enable_if_t<!std::is_same_v<
                            std::decay_t<T>, CopyableValueType>>>
  CopyableValueType(T &&v) : value(std::forward<T>(v)) {}

  CopyableValueType(const CopyableValueType &other)
      : value(yoriito::CloneValueType(other.value)) {}

  CopyableValueType &operator=(const CopyableValueType &other) {
    if (this != &other) {
      value = yoriito::CloneValueType(other.value);
    }
    return *this;
  }

  CopyableValueType(CopyableValueType &&) noexcept = default;
  CopyableValueType &operator=(CopyableValueType &&) noexcept = default;

  yoriito::ValueType value;
};

}  // namespace yoriito::test

#endif  // KURA_CLIENT_CC_COMMON_INC_TEST_COPYABLE_VALUE_TYPE_H_
