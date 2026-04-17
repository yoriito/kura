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

use crate::controller::yoriito::converter::FilterUtil;
use crate::interface::Interactor;
use crate::interface::Subscriber;
use crate::interface::signal as internal_signal;
use crate::interface::signal::MapExternalToKuraResult;
use crate::interface::signal::MapOptionToKuraError;
use crate::interface::subscription::{GetCondition, Mode, SubscriptionCondition};

use std::time::{SystemTime, UNIX_EPOCH};
use tokio::sync::mpsc;
use tokio::sync::mpsc::Sender;
use tokio_stream::wrappers::ReceiverStream;
use tokio_util::sync::CancellationToken;
use tracing::error;

use async_trait::async_trait;
use std::any::Any;
use std::pin::Pin;
use std::sync::Arc;

use tonic::{Response, Status};

use crate::controller::yoriito::common::viss as common_viss;
use crate::controller::yoriito::common::viss::consumer as consumer_viss;
use crate::controller::yoriito::common::viss::consumer::viss_server::Viss;

#[derive(Clone)]
pub struct YoriitoConsumerServer {
    addr: String,
    interactor: Arc<dyn Interactor + Send + Sync>,
}

impl std::fmt::Debug for YoriitoConsumerServer {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("YoriitoConsumerServer")
            .field("addr", &self.addr)
            .finish()
    }
}

impl YoriitoConsumerServer {
    pub fn new(
        addr: &str,
        interactor: Arc<dyn Interactor + Send + Sync>,
    ) -> internal_signal::KuraResult<Self> {
        Ok(Self {
            addr: addr.to_string(),
            interactor,
        })
    }

    pub async fn start(&self, cancel: CancellationToken) -> internal_signal::KuraResult<()> {
        let addr = self.addr.clone();
        let viss_server_impl: consumer_viss::viss_server::VissServer<YoriitoConsumerServer> =
            consumer_viss::viss_server::VissServer::new(self.clone());
        let server = tonic::transport::Server::builder().add_service(viss_server_impl);
        let addr = addr.parse().to_kura_result(
            internal_signal::KuraErrorCode::BadGateway,
            internal_signal::KuraErrorReason::BadGateway,
            format!("failed to parse {addr}").to_string(),
        )?;
        tokio::select! {
            res = server.serve_with_shutdown(addr, cancel.cancelled()) => res.to_kura_result(
                internal_signal::KuraErrorCode::Unspecified,
                internal_signal::KuraErrorReason::Unspecified,
                "failed to shutdown".to_string()
            )?,
            _ = cancel.cancelled() => {}
        }
        Ok(())
    }

    async fn set_single_signal(
        &self,
        _request: &consumer_viss::SetTargetRequest,
        data_package: &common_viss::DataPackageTarget,
    ) -> internal_signal::KuraResult<()> {
        let path = data_package.path.clone();
        let dp = data_package
            .dp
            .first()
            .to_kura_result(
                internal_signal::KuraErrorCode::NotFound,
                internal_signal::KuraErrorReason::InvalidData,
                "No DataPoint Found".to_string(),
            )?
            .into();
        self.interactor.set_target_vehicle_data(path, dp).await
    }
}

#[tonic::async_trait]
impl Viss for YoriitoConsumerServer {
    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "get(YoriitoConsumerServer)", level = "info", skip_all)]
    async fn get_current(
        &self,
        request: tonic::Request<consumer_viss::GetCurrentRequest>,
    ) -> Result<tonic::Response<consumer_viss::GetCurrentResponse>, tonic::Status> {
        let inner = request.into_inner();
        let paths_filter = inner.filter.and_then(|v| v.to_paths_filter());
        let condition = GetCondition {
            path: inner.path.clone(),
            paths_filter,
        };

        let current_signals = self.interactor.get_current_vehicle_data(condition);
        if let Ok(signals) = &current_signals
            && signals.is_empty()
        {
            let error = common_viss::Error {
                number: common_viss::ErrorCode::ErrorNotFound as i32,
                reason: common_viss::ErrorReason::ReasonInvalidData as i32,
                description: Some("not signals".to_string()),
            };
            let result = consumer_viss::get_current_response::Result::Error(error);
            let result = Some(result);
            let result = consumer_viss::GetCurrentResponse {
                ts: get_time() as i64,
                result,
            };
            return Ok(tonic::Response::new(result));
        }

        let response = match current_signals {
            Ok(vec) => {
                match vec
                    .into_iter()
                    .map(|signal| common_viss::DataPackageCurrent::try_from(&signal))
                    .collect::<Result<Vec<_>, _>>()
                {
                    Ok(data) => consumer_viss::get_current_response::Result::Data(
                        common_viss::DataPackagesCurrent { data },
                    ),
                    Err(error) => consumer_viss::get_current_response::Result::Error(
                        common_viss::Error::from(&error),
                    ),
                }
            }
            Err(error) => {
                consumer_viss::get_current_response::Result::Error(common_viss::Error::from(&error))
            }
        };
        let result = Some(response);
        Ok(tonic::Response::new(consumer_viss::GetCurrentResponse {
            ts: get_time() as i64,
            result,
        }))
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "inspect(YoriitoConsumerServer)", level = "info", skip_all)]
    async fn inspect(
        &self,
        request: tonic::Request<common_viss::InspectRequest>,
    ) -> std::result::Result<tonic::Response<common_viss::InspectResponse>, tonic::Status> {
        let inner = request.into_inner();
        let paths_filter = inner.filter.as_ref().and_then(|v| v.to_paths_filter());
        let condition = GetCondition {
            path: inner.path.clone(),
            paths_filter,
        };

        let signals = self.interactor.get_current_vehicle_data(condition);
        let signals = signals.map_err(|e| common_viss::Error::from(&e));
        let response = match signals {
            Ok(vec) => {
                match vec
                    .into_iter()
                    .map(|signal| common_viss::MetadataEntry::try_from(&signal))
                    .collect::<Result<Vec<_>, _>>()
                {
                    Ok(data) => common_viss::inspect_response::Result::Metadata(
                        common_viss::MetadataEntries { entries: data },
                    ),
                    Err(error) => common_viss::inspect_response::Result::Error(
                        common_viss::Error::from(&error),
                    ),
                }
            }
            Err(error) => common_viss::inspect_response::Result::Error(error),
        };

        let result = Some(response);
        Ok(tonic::Response::new(common_viss::InspectResponse {
            ts: get_time() as i64,
            result,
        }))
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "set(YoriitoConsumerServer)", level = "info", skip_all)]
    async fn set_target(
        &self,
        request: tonic::Request<consumer_viss::SetTargetRequest>,
    ) -> std::result::Result<tonic::Response<consumer_viss::SetTargetResponse>, tonic::Status> {
        let inner = request.into_inner();
        let data_packages = match &inner.data {
            Some(data) => &data.data,
            None => &[].to_vec(),
        };
        if data_packages.is_empty() {
            error!("data packages is None");
            let error = common_viss::Error {
                number: common_viss::ErrorCode::ErrorUnspecified as i32,
                reason: common_viss::ErrorReason::ReasonUnspecified as i32,
                description: Some("data packages is None".to_string()),
            };
            let result = consumer_viss::set_target_response::Result::Error(error);
            let result = Some(result);
            let result = consumer_viss::SetTargetResponse {
                ts: get_time() as i64,
                result,
            };
            return Ok(tonic::Response::new(result));
        }

        let futures = data_packages
            .iter()
            .map(async |data_package| self.set_single_signal(&inner, data_package).await);
        let results = futures::future::join_all(futures).await;
        let results: Vec<Result<(), common_viss::Error>> = results
            .into_iter()
            .map(|x| x.map_err(|error| common_viss::Error::from(&error)))
            .collect();
        let first_err = results.iter().find_map(|r| r.as_ref().err());
        if let Some(first_err) = first_err {
            let result = consumer_viss::set_target_response::Result::Error(first_err.clone());
            let result = Some(result);
            let result = consumer_viss::SetTargetResponse {
                ts: get_time() as i64,
                result,
            };
            return Ok(tonic::Response::new(result));
        }

        Ok(tonic::Response::new(consumer_viss::SetTargetResponse {
            ts: get_time() as i64,
            result: Some(consumer_viss::set_target_response::Result::Success(())),
        }))
    }

    type SubscribeCurrentStream = Pin<
        Box<
            dyn tonic::codegen::tokio_stream::Stream<
                    Item = std::result::Result<
                        consumer_viss::SubscribeCurrentResponse,
                        tonic::Status,
                    >,
                > + Send
                + 'static,
        >,
    >;

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "subscribe(YoriitoConsumerServer)", level = "info", skip_all)]
    async fn subscribe_current(
        &self,
        request: tonic::Request<consumer_viss::SubscribeCurrentRequest>,
    ) -> std::result::Result<tonic::Response<Self::SubscribeCurrentStream>, tonic::Status> {
        let inner = request.into_inner();
        let paths_filter = inner.filter.as_ref().and_then(|v| v.to_paths_filter());
        let misc_filter = inner.filter.as_ref().and_then(|v| v.to_misc_filter());
        let (sender, receiver) = mpsc::channel(10);
        let ret = self
            .interactor
            .subscribe(
                Arc::new(()),
                Arc::new(SubscribrYoriito {
                    sender: sender.clone(),
                }),
                SubscriptionCondition {
                    mode: Mode::Current,
                    path: inner.path,
                    paths_filter,
                    misc_filter,
                },
            )
            .await;
        if let Err(e) = ret {
            error!("failed to subscribe: {e}");
            let error = common_viss::Error::from(&e);
            let message = consumer_viss::subscribe_current_response::MessageType::Error(
                common_viss::SubscribeError { error: Some(error) },
            );
            let message_type = Some(message);
            let response = consumer_viss::SubscribeCurrentResponse {
                ts: get_time() as i64,
                message_type,
            };
            let _ = sender.send(Ok(response)).await;
        }

        type SubscribeRequestStream = Pin<
            Box<
                dyn tokio_stream::Stream<
                        Item = Result<consumer_viss::SubscribeCurrentResponse, Status>,
                    > + Send
                    + 'static,
            >,
        >;

        Ok(Response::new(
            Box::pin(ReceiverStream::new(receiver)) as SubscribeRequestStream
        ))
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "unsubscribe(YoriitoConsumerServer)", level = "info", skip_all)]
    async fn unsubscribe(
        &self,
        request: tonic::Request<common_viss::UnsubscribeRequest>,
    ) -> std::result::Result<tonic::Response<common_viss::UnsubscribeResponse>, tonic::Status> {
        let inner = request.into_inner();
        let result = self
            .interactor
            .unsubscribe(inner.subscription_id.clone())
            .await;
        let result = result.map_err(|error| common_viss::Error::from(&error));
        let result = match result {
            Ok(_) => common_viss::unsubscribe_response::Result::Success(()),
            Err(error) => common_viss::unsubscribe_response::Result::Error(error),
        };
        let result = Some(result);
        let payload = common_viss::UnsubscribeResponse {
            ts: get_time() as i64,
            result,
        };
        Ok(Response::new(payload))
    }
}

#[derive(Debug)]
struct SubscribrYoriito {
    sender: Sender<Result<consumer_viss::SubscribeCurrentResponse, Status>>,
}

#[async_trait]
impl Subscriber for SubscribrYoriito {
    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(
        name = "notify_subscribe_result(SubscribrYoriito)",
        level = "info",
        skip_all
    )]
    async fn notify_subscribe_result(
        &self,
        _object: Arc<dyn Any + Send + Sync>,
        subscription_id: String,
        ret: bool,
    ) -> internal_signal::KuraResult<()> {
        let message_type = if ret {
            let message = consumer_viss::subscribe_current_response::MessageType::Success(
                common_viss::SubscribeSuccess { subscription_id },
            );
            Some(message)
        } else {
            let error = common_viss::Error {
                number: common_viss::ErrorCode::ErrorNotFound as i32,
                reason: common_viss::ErrorReason::ReasonUnspecified as i32,
                description: None,
            };
            let message = consumer_viss::subscribe_current_response::MessageType::Error(
                common_viss::SubscribeError { error: Some(error) },
            );
            Some(message)
        };

        let response = consumer_viss::SubscribeCurrentResponse {
            ts: get_time() as i64,
            message_type,
        };

        self.sender.send(Ok(response)).await.to_kura_result(
            internal_signal::KuraErrorCode::BadGateway,
            internal_signal::KuraErrorReason::Unspecified,
            "send failed".to_string(),
        )
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "notify_signals(SubscribrYoriito)", level = "info", skip_all)]
    async fn notify_signals(
        &self,
        _object: Arc<dyn Any + Send + Sync>,
        subscription_id: String,
        signals: &[crate::interface::signal::Signal],
    ) -> internal_signal::KuraResult<()> {
        let data: internal_signal::KuraResult<Vec<common_viss::DataPackageCurrent>> = signals
            .iter()
            .map(common_viss::DataPackageCurrent::try_from)
            .collect();
        let data = Some(common_viss::DataPackagesCurrent { data: data? });
        let event = consumer_viss::SubscriptionCurrentEvent {
            subscription_id,
            data,
        };
        let message = consumer_viss::subscribe_current_response::MessageType::Event(event);
        let event = consumer_viss::SubscribeCurrentResponse {
            ts: get_time() as i64,
            message_type: Some(message),
        };

        self.sender.send(Ok(event)).await.to_kura_result(
            internal_signal::KuraErrorCode::BadGateway,
            internal_signal::KuraErrorReason::Unspecified,
            "send failed".to_string(),
        )
    }
}

fn get_time() -> u64 {
    let now = SystemTime::now();
    let duration_since_epoch = now.duration_since(UNIX_EPOCH).unwrap();
    duration_since_epoch.as_secs()
}
