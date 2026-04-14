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

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "grpc/viss_grpc_context_factory.h"
#include "viss_common_error.h"
#include "viss_consumer.h"
//

#include "grpc/grpc_interceptor_factory.h"
#include "grpc/viss_consumer_grpc_impl.h"
#include "grpc/viss_grpc_get_impl.h"
#include "grpc/viss_grpc_inspect_impl.h"
#include "grpc/viss_grpc_set_impl.h"
#include "grpc/viss_grpc_subscribe_context.h"
#include "grpc/viss_grpc_subscribe_impl.h"
#include "grpc/viss_grpc_unsubscribe_impl.h"
#include "grpc/viss_grpc_value_util.h"
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/consumer.grpc.pb.h"
#include "yoriito/viss/v1/types/value.grpc.pb.h"
//

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using yoriito::viss::v1::consumer::VISS;
namespace yoriito::databroker {

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

  auto channel = grpc::experimental::CreateCustomChannelWithInterceptors(
      target, grpc::InsecureChannelCredentials(), args,
      std::move(interceptor_creators));
  stub_ = VISS::NewStub(channel);

  return Result<void>::Ok();
}

Result<ConsumerGetResponse> GrpcConsumerImpl::Get(
    const ConsumerGetRequest& data) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::consumer::GetCurrentRequest request;
  yoriito::viss::v1::consumer::GetCurrentResponse response;
  std::unique_ptr<yoriito::databroker::consumer::VissGrpcGetImpl> impl =
      std::make_unique<yoriito::databroker::consumer::VissGrpcGetImpl>(
          stub_.get());

  impl->CreateGetRequest(request, data);

  Status status = impl->Execute(*context.get(), request, response);
  return impl->ConvertGetResponse(status, response, data);
}

Result<ConsumerInspectResponse> GrpcConsumerImpl::Inspect(
    const ConsumerInspectRequest& data) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::InspectRequest request;
  yoriito::viss::v1::InspectResponse response;

  std::unique_ptr<yoriito::databroker::consumer::VissGrpcInspectImpl> impl =
      std::make_unique<yoriito::databroker::consumer::VissGrpcInspectImpl>(
          stub_.get());

  impl->CreateInspectRequest(request, data);
  auto status = impl->Execute(*context.get(), request, response);
  return impl->ConvertInspectResponse(status, response, data);
}

Result<void> GrpcConsumerImpl::Set(const ConsumerSetRequest& data) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::consumer::SetTargetRequest request;
  yoriito::viss::v1::consumer::SetTargetResponse response;

  std::unique_ptr<yoriito::databroker::consumer::VissGrpcSetImpl> impl =
      std::make_unique<yoriito::databroker::consumer::VissGrpcSetImpl>(
          stub_.get());

  impl->CreateSetRequest(request, data);

  Status status = impl->Execute(*context.get(), request, response);

  return impl->ConvertSetResponse(status, response, data);
}

Result<std::unique_ptr<ConsumerSubscribeContext>> GrpcConsumerImpl::Subscribe(
    const ConsumerSubscribeRequest& data) {
  auto context = ClientContextFactory::Create(
      ClientContextFactory::Config{/*timeout_seconds*/ 0});
  yoriito::viss::v1::consumer::SubscribeCurrentResponse response;
  yoriito::viss::v1::consumer::SubscribeCurrentRequest request;

  std::unique_ptr<yoriito::databroker::consumer::VissGrpcSubscribeImpl> impl =
      std::make_unique<yoriito::databroker::consumer::VissGrpcSubscribeImpl>(
          stub_.get());

  impl->CreateSubscribeRequest(request, data);

  auto reader = impl->Execute(*context, request);

  return impl->ConvertSubscribeResponse(context, std::move(reader), data);
}

Result<void> GrpcConsumerImpl::UnSubscribe(
    std::unique_ptr<ConsumerSubscribeContext> subscription_id) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::UnsubscribeRequest request;
  yoriito::viss::v1::UnsubscribeResponse response;
  auto raw_ptr = subscription_id.get();

  std::unique_ptr<yoriito::databroker::consumer::VissGrpcUnSubscribeImpl> impl =
      std::make_unique<yoriito::databroker::consumer::VissGrpcUnSubscribeImpl>(
          stub_.get());

  auto viss_context =
      dynamic_cast<yoriito::databroker::consumer::VissGrpcSubscribeContext*>(
          raw_ptr);
  if (viss_context) {
    viss_context->Cancel();
    impl->CreateUnsubscribeRequest(*viss_context, request);

    auto result = impl->Execute(*context.get(), request, response);

    return impl->ConvertUnsubscribeResponse(result, response);
  } else {
    std::string error_message = "subscription id is invalid";
    VISS_LOG_ERROR("CNSR_UNSUBSCRIBE", error_message);
    return Result<void>::Err(
        VissCommonError::CreateIllegalError(error_message));
  }
}

}  // namespace yoriito::databroker
