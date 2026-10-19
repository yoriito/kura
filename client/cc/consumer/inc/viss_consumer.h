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

#ifndef CONSUMER_INC_VISS_CONSUMER_H_
#define CONSUMER_INC_VISS_CONSUMER_H_

#include <cstdlib>
#include <memory>
#include <string>

#include "consumer/inc/viss_consumer_factory.h"
#include "consumer/inc/viss_consumer_impl.h"
#include "databroker/databroker_consumer.h"
#include "yoriito/yoriito_log.h"

namespace yoriito::databroker {
class VissConsumer : public DataBrokerConsumer {
 public:
  static DataBrokerConsumer* GetInstance() {
    static VissConsumer instance;
    return static_cast<DataBrokerConsumer*>(&instance);
  }

  static constexpr const char* kConnectionInfo = "127.0.0.1:50052";
  VissConsumer()
      : DataBrokerConsumer(),
        impl_(VissConsumerFactory::CreateConsumer(
            VissConsumerFactory::ConsumerType::kGrpc)) {
    const char* grpc_target = std::getenv("YORIITO_CONSUMER_ENDPOINT");
    if ((grpc_target == nullptr) || (std::string(grpc_target).length() == 0)) {
      grpc_target = kConnectionInfo;
    }
    auto result = impl_->Connect(grpc_target);
    if (result.IsErr()) {
      YORIITO_LOG_ERROR("VissConsumer", "Failed to connect to the server.");
    }
  }
  ~VissConsumer() = default;

  /**
   * @ingroup consumer
   * @fn Result<ConsumerGetResponse> Get(const ConsumerGetRequest& request)
   * override
   * @brief Retrieves one or more datapoint values from the DataBroker.
   * @deprecated -
   * @pre -
   * @param request The request containing information about the datapoints to
   * retrieve.
   * @retval Result<ConsumerGetResponse> Returns the result with datapoint
   * values on success (returns empty data if the data is not set), or error
   * information on failure.
   * @retval ERROR_NOT_FOUND : The requested path was not found.( if the path is
   * completely not found )
   * @retval ERROR_CONNREFUSED: Unable to connect to the server and grpc error
   * occurred
   * @note -
   * @code -
   */
  Result<ConsumerGetResponse> Get(const ConsumerGetRequest& request) override;

  /**
   * @ingroup consumer
   * @fn Result<ConsumerInspectResponse> Inspect(const ConsumerInspectRequest&
   * request) override
   * @brief Inspects metadata or status of datapoints in the DataBroker.
   * @deprecated -
   * @pre -
   * @param request The request specifying which datapoints to inspect.
   * @retval Result<ConsumerInspectResponse> Returns the inspection result on
   * success, or error information on failure.
   * @retval ERROR_NOT_FOUND : The requested path was not found.( if the path is
   * completely not found )
   * @retval ERROR_INVAL : The response is illegal.
   * @retval ERROR_CONNREFUSED: Unable to connect to the server and grpc error
   * occurred
   * @note -
   * @code -
   */
  Result<ConsumerInspectResponse> Inspect(
      const ConsumerInspectRequest& request) override;

  /**
   * @ingroup consumer
   * @fn Result<void> Set(const ConsumerSetRequest& data) override
   * @brief Sets one or more datapoint values in the DataBroker.
   * @deprecated -
   * @pre -
   * @param data The request containing datapoints and values to set.
   * @retval Result<void> Returns void on success, or error information on
   * failure.
   * @retval ERROR_NOT_FOUND : The requested path was not found
   * @retval ERROR_BAD_REQUEST: The request is malformed.( set to sensor ,
   * datatype inconsistent)
   * @retval ERROR_CONNREFUSED: Unable to connect to the server and grpc error
   * occurred
   * @note -
   * @code -
   */
  Result<void> Set(const ConsumerSetRequest& data) override;

  /**
   * @ingroup consumer
   * @fn Result<std::unique_ptr<ConsumerSubscribeContext>> Subscribe(const
   * yoriito::databroker::ConsumerSubscribeRequest& request) override
   * @brief Subscribes to updates for one or more datapoints in the DataBroker.
   * @deprecated -
   * @pre -
   * @param request The request specifying which datapoints to subscribe to.
   * @retval Result<std::unique_ptr<ConsumerSubscribeContext>> Returns the
   * subscription context on success, or error information on failure.
   * @retval ERROR_NOT_FOUND : The requested path was not found
   * @retval ERROR_INVAL : The response is illegal
   * @retval ERROR_CONNREFUSED: Unable to connect to the server and grpc error
   * occurred
   * @note May use callback for asynchronous notification.
   * @code -
   */
  Result<std::unique_ptr<ConsumerSubscribeContext>> Subscribe(
      const yoriito::databroker::ConsumerSubscribeRequest& data) override;

  /**
   * @ingroup consumer
   * @fn Result<void>
   * UnSubscribe(std::unique_ptr<yoriito::databroker::ConsumerSubscribeContext>
   * subscription_id) override
   * @brief Unsubscribes from updates for datapoints in the DataBroker.
   * @deprecated -
   * @pre -
   * @param subscription_id The subscription context to cancel.
   * @retval Result<void> Returns void on success, or error information on
   * failure.
   * @retval ERROR_CONNREFUSED: Unable to connect to the server and grpc error
   * occurred
   * @note -
   * @code -
   */
  Result<void> UnSubscribe(
      std::unique_ptr<yoriito::databroker::ConsumerSubscribeContext>
          subscription_id) override;

 private:
  std::unique_ptr<VissConsumerImpl> impl_;
};
}  // namespace yoriito::databroker

#endif  // CONSUMER_INC_VISS_CONSUMER_H_
