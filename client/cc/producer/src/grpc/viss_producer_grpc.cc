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

#include <string>

#include "viss_common_error.h"
#include "viss_producer.h"

//
#include "grpc/grpc_interceptor_factory.h"
#include "grpc/viss_grpc_context_factory.h"
#include "grpc/viss_grpc_define_impl.h"
#include "grpc/viss_grpc_delete_impl.h"
#include "grpc/viss_grpc_get_impl.h"
#include "grpc/viss_grpc_inspect_impl.h"
#include "grpc/viss_grpc_set_impl.h"
#include "grpc/viss_grpc_subscribe_impl.h"
#include "grpc/viss_grpc_unsubscribe_impl.h"
#include "grpc/viss_grpc_value_util.h"
#include "grpc/viss_producer_grpc_impl.h"
#include "grpcpp/grpcpp.h"
#include "yoriito/viss/v1/producer.grpc.pb.h"
#include "yoriito/viss/v1/producer.pb.h"
#include "yoriito/viss/v1/types/value.grpc.pb.h"
//

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using yoriito::viss::v1::producer::VISS;
namespace yoriito::databroker {

Result<void> GrpcProducerImpl::Connect(std::string target) {
  grpc::ChannelArguments args;
  args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 30000);
  args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
  args.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 0);
  std::vector<
      std::unique_ptr<grpc::experimental::ClientInterceptorFactoryInterface>>
      interceptor_creators;
  interceptor_creators.push_back(std::make_unique<GrpcInterceptorFactory>(
      yoriito::databroker::DataBrokerType::kProducer));

  auto channel = grpc::experimental::CreateCustomChannelWithInterceptors(
      target, grpc::InsecureChannelCredentials(), args,
      std::move(interceptor_creators));
  stub_ = VISS::NewStub(channel);
  return Result<void>::Ok();
}

Result<yoriito::databroker::ProducerGetResponse> GrpcProducerImpl::Get(
    const yoriito::databroker::ProducerGetRequest& data) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::producer::GetTargetRequest request;
  yoriito::viss::v1::producer::GetTargetResponse response;
  std::unique_ptr<yoriito::databroker::producer::VissGrpcGetImpl> impl =
      std::make_unique<yoriito::databroker::producer::VissGrpcGetImpl>(
          stub_.get());

  impl->CreateGetRequest(request, data);

  Status status = impl->Execute(*context.get(), request, response);
  return impl->ConvertGetResponse(status, response, data);
}

Result<yoriito::databroker::ProducerInspectResponse> GrpcProducerImpl::Inspect(
    const yoriito::databroker::ProducerInspectRequest& data) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::InspectRequest request;
  yoriito::viss::v1::InspectResponse response;

  std::unique_ptr<yoriito::databroker::producer::VissGrpcInspectImpl> impl =
      std::make_unique<yoriito::databroker::producer::VissGrpcInspectImpl>(
          stub_.get());

  impl->CreateInspectRequest(request, data);
  auto status = impl->Execute(*context.get(), request, response);
  return impl->ConvertInspectResponse(status, response);
}

Result<void> GrpcProducerImpl::Set(const ProducerSetRequest& data) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::producer::SetCurrentRequest request;
  yoriito::viss::v1::producer::SetCurrentResponse response;

  std::unique_ptr<yoriito::databroker::producer::VissGrpcSetImpl> impl =
      std::make_unique<yoriito::databroker::producer::VissGrpcSetImpl>(
          stub_.get());

  impl->CreateSetRequest(request, data);

  Status status = impl->Execute(*context.get(), request, response);

  return impl->ConvertSetResponse(status, response);
}

Result<std::unique_ptr<ProducerSubscribeContext>> GrpcProducerImpl::Subscribe(
    const ProducerSubscribeRequest& data) {
  auto context = ClientContextFactory::Create(
      ClientContextFactory::Config{/*timeout_seconds*/ 0});
  yoriito::viss::v1::producer::SubscribeTargetResponse response;
  yoriito::viss::v1::producer::SubscribeTargetRequest request;

  //
  std::unique_ptr<yoriito::databroker::producer::VissGrpcSubscribeImpl> impl =
      std::make_unique<yoriito::databroker::producer::VissGrpcSubscribeImpl>(
          stub_.get());
  //
  impl->CreateSubscribeRequest(request, data);
  //
  auto reader = impl->Execute(*context.get(), request);
  //
  return impl->ConvertSubscribeResponse(context, std::move(reader));
}

Result<void> GrpcProducerImpl::UnSubscribe(
    std::unique_ptr<ProducerSubscribeContext> subscription_id) {
  auto raw_ptr = subscription_id.get();

  std::unique_ptr<yoriito::databroker::producer::VissGrpcUnSubscribeImpl> impl =
      std::make_unique<yoriito::databroker::producer::VissGrpcUnSubscribeImpl>(
          stub_.get());

  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::UnsubscribeRequest request;
  yoriito::viss::v1::UnsubscribeResponse response;

  auto viss_context =
      dynamic_cast<yoriito::databroker::producer::VissGrpcSubscribeContext*>(
          raw_ptr);
  if (viss_context) {
    viss_context->Cancel();
    impl->CreateUnsubscribeRequest(*viss_context, request);

    auto result = impl->Execute(*context.get(), request, response);

    return impl->ConvertUnsubscribeResponse(result, response);
  } else {
    std::string error_message = "subscription id is invalid";
    VISS_LOG_ERROR("PRDR_UNSUBSCRIBE", error_message);
    return Result<void>::Err(
        VissCommonError::CreateIllegalError(error_message));
  }
}

Result<void> GrpcProducerImpl::Define(const ProducerDefineRequest& data) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::producer::DefineRequest request;
  yoriito::viss::v1::producer::DefineResponse response;
  std::unique_ptr<yoriito::databroker::producer::VissGrpcDefineImpl> impl =
      std::make_unique<yoriito::databroker::producer::VissGrpcDefineImpl>(
          stub_.get());

  impl->CreateDefineRequest(request, data);

  Status status = impl->Execute(*context.get(), request, response);

  return impl->ConvertDefineResponse(status, response);
}

Result<void> GrpcProducerImpl::Delete(const ProducerDeleteRequest& data) {
  auto context = ClientContextFactory::Create(ClientContextFactory::Config{});
  yoriito::viss::v1::producer::DeleteRequest request;
  yoriito::viss::v1::producer::DeleteResponse response;

  std::unique_ptr<yoriito::databroker::producer::VissGrpcDeleteImpl> impl =
      std::make_unique<yoriito::databroker::producer::VissGrpcDeleteImpl>(
          stub_.get());

  impl->CreateDeleteRequest(request, data);

  Status status = impl->Execute(*context.get(), request, response);

  return impl->ConvertDeleteResponse(status, response);
}
}  // namespace yoriito::databroker
