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

#include "grpc/viss_consumer_grpc_impl.h"
//
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common/inc/viss_common_error.h"
#include "consumer/inc/viss_consumer.h"
#include "grpc/grpc_interceptor_factory.h"
#include "grpc/viss_grpc_context_factory.h"
#include "grpc/viss_grpc_get_impl.h"
#include "grpc/viss_grpc_inspect_impl.h"
#include "grpc/viss_grpc_set_impl.h"
#include "grpc/viss_grpc_subscribe_impl.h"
#include "grpc/viss_grpc_unsubscribe_impl.h"
#include "grpc/viss_grpc_value_util.h"

namespace yoriito::databroker {

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using yoriito::viss::v1::consumer::VISS;

Result<void> GrpcConsumerImpl::Connect(std::string target) {
  grpc::ChannelArguments args;
  args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 30000);
  args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
  args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 0);
  std::vector<
      std::unique_ptr<grpc::experimental::ClientInterceptorFactoryInterface>>
      interceptor_creators;
  interceptor_creators.push_back(std::make_unique<GrpcInterceptorFactory>(
      yoriito::databroker::DataBrokerType::kConsumer));

  std::shared_ptr<grpc::ChannelCredentials> credentials =
      grpc::InsecureChannelCredentials();
  std::shared_ptr<Channel> channel =
      grpc::experimental::CreateCustomChannelWithInterceptors(
          target, credentials, args, std::move(interceptor_creators));
  stub_ = VISS::NewStub(channel);
  return Result<void>::Ok();
}

Result<ConsumerGetResponse> GrpcConsumerImpl::Get(
    const ConsumerGetRequest& request) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::consumer::GetCurrentRequest viss_request;
  yoriito::viss::v1::consumer::GetCurrentResponse viss_response;
  std::unique_ptr<yoriito::databroker::consumer::VissGrpcGetImpl> impl =
      std::make_unique<yoriito::databroker::consumer::VissGrpcGetImpl>(
          stub_.get());

  impl->CreateGetRequest(viss_request, request);

  Status status = impl->Execute(*context.get(), viss_request, viss_response);
  return impl->ConvertGetResponse(std::move(status), viss_response, request);
}

Result<ConsumerInspectResponse> GrpcConsumerImpl::Inspect(
    const ConsumerInspectRequest& request) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::InspectRequest viss_request;
  yoriito::viss::v1::InspectResponse viss_response;

  std::unique_ptr<yoriito::databroker::consumer::VissGrpcInspectImpl> impl =
      std::make_unique<yoriito::databroker::consumer::VissGrpcInspectImpl>(
          stub_.get());

  impl->CreateInspectRequest(viss_request, request);
  auto status = impl->Execute(*context.get(), viss_request, viss_response);
  return impl->ConvertInspectResponse(std::move(status), viss_response,
                                      request);
}

Result<void> GrpcConsumerImpl::Set(const ConsumerSetRequest& data) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::consumer::SetTargetRequest viss_request;
  yoriito::viss::v1::consumer::SetTargetResponse viss_response;

  std::unique_ptr<yoriito::databroker::consumer::VissGrpcSetImpl> impl =
      std::make_unique<yoriito::databroker::consumer::VissGrpcSetImpl>(
          stub_.get());

  impl->CreateSetRequest(viss_request, data);

  Status status = impl->Execute(*context.get(), viss_request, viss_response);

  return impl->ConvertSetResponse(std::move(status), viss_response, data);
}

Result<std::unique_ptr<ConsumerSubscribeContext>> GrpcConsumerImpl::Subscribe(
    const ConsumerSubscribeRequest& data) {
  auto context = ClientContextFactory::Create(
      ClientContextFactory::Config{/*timeout_seconds*/ 0});
  yoriito::viss::v1::consumer::SubscribeCurrentResponse viss_response;
  yoriito::viss::v1::consumer::SubscribeCurrentRequest viss_request;

  std::unique_ptr<yoriito::databroker::consumer::VissGrpcSubscribeImpl> impl =
      std::make_unique<yoriito::databroker::consumer::VissGrpcSubscribeImpl>(
          stub_.get());

  impl->CreateSubscribeRequest(viss_request, data);
  auto reader = impl->Execute(*context, viss_request);
  return impl->ConvertSubscribeResponse(context, std::move(reader), data);
}

Result<void> GrpcConsumerImpl::UnSubscribe(
    std::unique_ptr<ConsumerSubscribeContext> subscription_id) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::UnsubscribeRequest viss_request;
  yoriito::viss::v1::UnsubscribeResponse viss_response;
  auto raw_ptr = subscription_id.get();

  std::unique_ptr<yoriito::databroker::consumer::VissGrpcUnSubscribeImpl> impl =
      std::make_unique<yoriito::databroker::consumer::VissGrpcUnSubscribeImpl>(
          stub_.get());

  auto viss_context =
      dynamic_cast<yoriito::databroker::consumer::VissGrpcSubscribeContext*>(
          raw_ptr);
  if (viss_context != nullptr) {
    viss_context->Cancel();
    impl->CreateUnsubscribeRequest(*viss_context, viss_request);

    auto result = impl->Execute(*context.get(), viss_request, viss_response);

    return impl->ConvertUnsubscribeResponse(std::move(result), viss_response);
  } else {
    std::string error_message = "subscription id is invalid";
    YORIITO_LOG_ERROR("CNSR_UNSUBSCRIBE", error_message);
    return Result<void>::Err(
        VissCommonError::CreateIllegalError(error_message));
  }
}

}  // namespace yoriito::databroker
