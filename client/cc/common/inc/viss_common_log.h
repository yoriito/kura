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

#ifndef KURA_CLIENT_CC_COMMON_INC_VISS_COMMON_LOG_H_
#define KURA_CLIENT_CC_COMMON_INC_VISS_COMMON_LOG_H_

#include "yoriito_log.h"

#define VISS_LOG_DEBUG(tag, message) YORIITO_LOG_DEBUG(tag, message)
#define VISS_LOG_INFO(tag, message) YORIITO_LOG_INFO(tag, message)
#define VISS_LOG_WARN(tag, message) YORIITO_LOG_WARN(tag, message)
#define VISS_LOG_ERROR(tag, message) YORIITO_LOG_ERROR(tag, message)

#endif  // KURA_CLIENT_CC_COMMON_INC_VISS_COMMON_LOG_H_
