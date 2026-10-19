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

use crate::common::get_time;
use crate::controller::yoriito::converter::FilterUtil;
use crate::controller::yoriito::converter::VissErrorExt;
use crate::interface::{
    model,
    result::{self, MapExternalToKuraResult, MapOptionToKuraError},
    subscription, traits,
};

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
use crate::controller::yoriito::common::viss::producer as producer_viss;
use crate::controller::yoriito::common::viss::producer::viss_server::Viss;

#[derive(Clone)]
pub struct YoriitoProducerServer {
    addr: String,
    interactor: Arc<dyn traits::Interactor + Send + Sync>,
}

impl std::fmt::Debug for YoriitoProducerServer {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("YoriitoProducerServer")
            .field("addr", &self.addr)
            .finish()
    }
}

impl YoriitoProducerServer {
    pub fn new(
        addr: &str,
        interactor: Arc<dyn traits::Interactor + Send + Sync>,
    ) -> result::KuraResult<Self> {
        Ok(Self {
            addr: addr.to_string(),
            interactor,
        })
    }
    pub async fn start(&self, cancel: CancellationToken) -> result::KuraResult<()> {
        let addr = self.addr.clone();
        let viss_server_impl: producer_viss::viss_server::VissServer<YoriitoProducerServer> =
            producer_viss::viss_server::VissServer::new(self.clone());
        let server = tonic::transport::Server::builder().add_service(viss_server_impl);
        let addr = addr.parse().to_kura_result(
            result::KuraErrorCode::BadGateway,
            result::KuraErrorReason::BadGateway,
            format!("parse {addr} failed"),
        )?;
        tokio::select! {
            res = server.serve_with_shutdown(addr, cancel.cancelled()) => res.to_kura_result(
                result::KuraErrorCode::Unspecified,
                result::KuraErrorReason::Unspecified,
                "shutdown failed".to_string(),
            )?,
            _ = cancel.cancelled() => {}
        }
        Ok(())
    }

    async fn set_single_signal(
        &self,
        _request: &producer_viss::SetCurrentRequest,
        data_package: &common_viss::DataPackageCurrent,
    ) -> result::KuraResult<()> {
        let path = data_package.path.clone();
        let datapoint = data_package.dp.first().to_kura_result(
            result::KuraErrorCode::Unspecified,
            result::KuraErrorReason::Unspecified,
            "invalid value: None".to_string(),
        )?;
        self.interactor
            .set_actual_state(path, datapoint.into())
            .await
    }

    async fn define_metadata_entries(
        &self,
        metadata_entries: Vec<common_viss::MetadataEntry>,
    ) -> producer_viss::define_response::Result {
        let mut result = producer_viss::define_response::Result::Success(());
        for entry in metadata_entries {
            let condition = subscription::GetCondition {
                path: entry.path.clone(),
                paths_filter: None, // define filters are defined, but not used in the proto.
            };
            let metadata_views = match self.interactor.get_metadata(condition).to_viss_error() {
                Ok(signals) => signals,
                Err(_) => [].to_vec(),
            };

            let leaf_type = model::LeafType::try_from(entry.r#type)
                .map_err(|error| common_viss::Error::from(&error));
            let leaf_type = match leaf_type {
                Ok(leaf_type) => leaf_type,
                Err(error) => {
                    result = producer_viss::define_response::Result::Error(error);
                    break;
                }
            };

            let data_type = match common_viss::DataType::try_from(entry.datatype) {
                Ok(data_type) => data_type,
                Err(_) => {
                    let error = result::KuraError {
                        number: result::KuraErrorCode::BadRequest,
                        reason: result::KuraErrorReason::InvalidData,
                        description: Some(format!("invalid data type: {}", entry.datatype)),
                    };
                    result = producer_viss::define_response::Result::Error(
                        common_viss::Error::from(&error),
                    );
                    break;
                }
            };
            let data_type = match model::ValueType::try_from((entry.datatype_path, data_type)) {
                Ok(data_type) => data_type,
                Err(error) => {
                    result = producer_viss::define_response::Result::Error(
                        common_viss::Error::from(&error),
                    );
                    break;
                }
            };
            let array_size = entry.arraysize;

            let mut single_metadata = match metadata_views.first() {
                Some(metadata) => metadata.metadata.clone(),
                None => model::SignalMetadata {
                    leaf_type,
                    data_type,
                    deprecation: entry.deprecated,
                    unit: entry.unit,
                    min: entry.min.as_ref().map(model::Value::from),
                    max: entry.max.as_ref().map(model::Value::from),
                    description: Some(entry.description.clone()),
                    comment: entry.comment.clone(),
                    allowd: match entry.allowed.clone() {
                        Some(allowd) => Some(allowd.values),
                        None => None,
                    },
                    default: entry.default.clone().map(|default| (&default).into()),
                    array_size,
                    user_defined_items: None,
                    pattern: None,
                },
            };

            // NOTE: users not allowed to overlay these configs
            // config.leaf_type
            // config.data_type
            // config.deprecation
            // config.unit
            single_metadata.min = entry.min.as_ref().map(model::Value::from);
            single_metadata.max = entry.max.as_ref().map(model::Value::from);
            single_metadata.description = Some(entry.description);
            single_metadata.comment = entry.comment;
            single_metadata.allowd = match entry.allowed {
                Some(allowd) => Some(allowd.values),
                None => None,
            };
            single_metadata.default = entry.default.map(|default| (&default).into());
            result = match self
                .interactor
                .update_metadata(&entry.path, &single_metadata)
                .await
                .map_err(|error| common_viss::Error::from(&error))
            {
                Ok(_) => producer_viss::define_response::Result::Success(()),
                Err(error) => producer_viss::define_response::Result::Error(error),
            };
        }
        result
    }

    async fn define_struct_metadata_entries(
        &self,
        struct_metadata_entries: Vec<common_viss::StructMetadataEntry>,
        mut result: producer_viss::define_response::Result,
    ) -> producer_viss::define_response::Result {
        if !struct_metadata_entries.is_empty() {
            let mut structs_to_register: Vec<model::VSSUserDefinedType> = Vec::new();

            for entry in struct_metadata_entries {
                let mut children = std::collections::HashMap::new();

                for prop_entry in &entry.properties {
                    let property_path = format!("{}.{}", entry.path, prop_entry.name);
                    let data_type = match common_viss::DataType::try_from(prop_entry.datatype) {
                        Ok(data_type) => data_type,
                        Err(_) => {
                            let error = result::KuraError {
                                number: result::KuraErrorCode::BadRequest,
                                reason: result::KuraErrorReason::InvalidData,
                                description: Some(format!(
                                    "invalid data type: {}",
                                    prop_entry.datatype
                                )),
                            };
                            result = producer_viss::define_response::Result::Error(
                                common_viss::Error::from(&error),
                            );
                            break;
                        }
                    };
                    let data_type = match model::ValueType::try_from((
                        prop_entry.datatype_path.clone(),
                        data_type,
                    )) {
                        Ok(dt) => dt,
                        Err(error) => {
                            result = producer_viss::define_response::Result::Error(
                                common_viss::Error::from(&error),
                            );
                            break;
                        }
                    };

                    if matches!(result, producer_viss::define_response::Result::Error(_)) {
                        break;
                    }

                    let metadata = model::PropertyMetadata {
                        leaf_type: model::LeafType::Attribute,
                        data_type,
                        deprecation: prop_entry.deprecated.clone(),
                        unit: prop_entry.unit.clone(),
                        min: prop_entry.min.as_ref().map(model::Value::from),
                        max: prop_entry.max.as_ref().map(model::Value::from),
                        description: Some(prop_entry.description.clone()),
                        comment: prop_entry.comment.clone(),
                        allowd: prop_entry.allowed.as_ref().map(|a| a.values.clone()),
                        default: prop_entry.default.as_ref().map(model::Value::from),
                        array_size: prop_entry.arraysize,
                        user_defined_items: None,
                        pattern: None,
                    };

                    children.insert(property_path, metadata);
                }

                if matches!(result, producer_viss::define_response::Result::Error(_)) {
                    break;
                }

                let struct_metadata = model::VSSUserDefinedType {
                    name: entry.path.clone(),
                    children,
                    description: None,
                    deprecation: None,
                    comment: None,
                };

                structs_to_register.push(struct_metadata);
            }
            if !matches!(result, producer_viss::define_response::Result::Error(_))
                && !structs_to_register.is_empty()
            {
                result = match self
                    .interactor
                    .initialize_vss_user_defined_types(structs_to_register)
                    .await
                    .map_err(|error| common_viss::Error::from(&error))
                {
                    Ok(_) => producer_viss::define_response::Result::Success(()),
                    Err(error) => producer_viss::define_response::Result::Error(error),
                };
            }
        }
        result
    }
}

#[tonic::async_trait]
impl Viss for YoriitoProducerServer {
    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "get_target(YoriitoProducerServer)", level = "info", skip_all)]
    async fn get_target(
        &self,
        request: tonic::Request<producer_viss::GetTargetRequest>,
    ) -> std::result::Result<tonic::Response<producer_viss::GetTargetResponse>, tonic::Status> {
        let inner = request.into_inner();
        let paths_filter = inner.filter.as_ref().and_then(|v| v.to_paths_filter());
        let condition = subscription::GetCondition {
            path: inner.path.clone(),
            paths_filter,
        };
        let signals = self.interactor.get_requested_state(condition);
        let signals = match signals {
            Ok(signals) => signals,
            Err(error) => {
                let result = producer_viss::get_target_response::Result::Error(
                    common_viss::Error::from(&error),
                );
                let result = Some(result);
                let result = producer_viss::GetTargetResponse {
                    ts: get_time() as i64,
                    result,
                };
                return Ok(tonic::Response::new(result));
            }
        };

        if signals.is_empty() {
            let error = common_viss::Error {
                number: common_viss::ErrorCode::ErrorNotFound as i32,
                reason: common_viss::ErrorReason::ReasonInvalidData as i32,
                description: Some("not signals".to_string()),
            };
            let result = producer_viss::get_target_response::Result::Error(error);
            let result = Some(result);
            let result = producer_viss::GetTargetResponse {
                ts: get_time() as i64,
                result,
            };
            return Ok(tonic::Response::new(result));
        }

        let data_packages_target: result::KuraResult<Vec<common_viss::DataPackageTarget>> =
            signals.iter().map(|signal| signal.try_into()).collect();
        let data_packages_target = data_packages_target
            .map(|value| common_viss::DataPackagesTarget { data: value })
            .map_err(|error| common_viss::Error::from(&error));
        let result = match data_packages_target {
            Ok(value) => producer_viss::get_target_response::Result::Data(value),
            Err(error) => producer_viss::get_target_response::Result::Error(error),
        };
        let result = Some(result);

        Ok(tonic::Response::new(producer_viss::GetTargetResponse {
            ts: get_time() as i64,
            result,
        }))
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "set_current(YoriitoProducerServer)", level = "info", skip_all)]
    async fn set_current(
        &self,
        request: tonic::Request<producer_viss::SetCurrentRequest>,
    ) -> std::result::Result<tonic::Response<producer_viss::SetCurrentResponse>, tonic::Status>
    {
        let inner = request.into_inner();
        let data_packages = match &inner.data {
            Some(common_viss::DataPackagesCurrent { data }) => data,
            None => &[].to_vec(),
        };
        if data_packages.is_empty() {
            error!("data packages is None");
            let error = common_viss::Error {
                number: common_viss::ErrorCode::ErrorUnspecified as i32,
                reason: common_viss::ErrorReason::ReasonUnspecified as i32,
                description: Some("data packages is None".to_string()),
            };
            let result = producer_viss::set_current_response::Result::Error(error);
            let result = Some(result);
            let result = producer_viss::SetCurrentResponse {
                ts: get_time() as i64,
                result,
            };
            return Ok(tonic::Response::new(result));
        }

        let futures = data_packages
            .iter()
            .map(async |data_package| self.set_single_signal(&inner, data_package).await);
        let results = futures::future::join_all(futures).await;
        let first_error = results.iter().find_map(|x| x.as_ref().err());
        let result = match first_error {
            Some(error) => {
                producer_viss::set_current_response::Result::Error(common_viss::Error::from(error))
            }
            None => producer_viss::set_current_response::Result::Success(()),
        };
        let result = Some(result);
        Ok(tonic::Response::new(producer_viss::SetCurrentResponse {
            ts: get_time() as i64,
            result,
        }))
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "inspect(YoriitoProducerServer)", level = "info", skip_all)]
    async fn inspect(
        &self,
        request: tonic::Request<common_viss::InspectRequest>,
    ) -> std::result::Result<tonic::Response<common_viss::InspectResponse>, tonic::Status> {
        let inner = request.into_inner();
        let paths_filter = inner.filter.as_ref().and_then(|v| v.to_paths_filter());
        let condition = subscription::GetCondition {
            path: inner.path.clone(),
            paths_filter,
        };
        let signals = self.interactor.get_metadata(condition);
        let signals = signals.map_err(|error| common_viss::Error::from(&error));
        let result = match signals {
            Ok(values) => {
                let filtered_defined_types = self
                    .interactor
                    .get_vss_user_defined_types_referenced_by(&values)
                    .unwrap_or_default();

                let metadata = values
                    .iter()
                    .map(common_viss::MetadataEntry::try_from)
                    .collect::<result::KuraResult<Vec<_>>>()
                    .map_err(|error| common_viss::Error::from(&error));

                match metadata {
                    Ok(metadata) => {
                        match common_viss::MetadataEntries::try_from(&filtered_defined_types) {
                            Ok(struct_entries) => common_viss::inspect_response::Result::Metadata(
                                common_viss::MetadataEntries {
                                    metadata_entries: metadata,
                                    struct_metadata_entries: struct_entries.struct_metadata_entries,
                                },
                            ),
                            Err(error) => common_viss::inspect_response::Result::Error(
                                common_viss::Error::from(&error),
                            ),
                        }
                    }
                    Err(error) => common_viss::inspect_response::Result::Error(error),
                }
            }
            Err(error) => common_viss::inspect_response::Result::Error(error),
        };
        let result = Some(result);
        Ok(tonic::Response::new(common_viss::InspectResponse {
            ts: get_time() as i64,
            result,
        }))
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "define(YoriitoProducerServer)", level = "info", skip_all)]
    async fn define(
        &self,
        request: tonic::Request<producer_viss::DefineRequest>,
    ) -> std::result::Result<tonic::Response<producer_viss::DefineResponse>, tonic::Status> {
        let inner: producer_viss::DefineRequest = request.into_inner();
        let entries = match inner.entries {
            Some(entries) => entries,
            None => {
                let error = result::KuraError {
                    number: result::KuraErrorCode::BadRequest,
                    reason: result::KuraErrorReason::InvalidData,
                    description: Some("metadata entries are required".to_string()),
                };
                let result =
                    producer_viss::define_response::Result::Error(common_viss::Error::from(&error));
                return Ok(tonic::Response::new(producer_viss::DefineResponse {
                    ts: get_time() as i64,
                    result: Some(result),
                }));
            }
        };
        let mut result = self
            .define_metadata_entries(entries.metadata_entries.clone())
            .await;
        result = self
            .define_struct_metadata_entries(entries.struct_metadata_entries, result)
            .await;

        Ok(tonic::Response::new(producer_viss::DefineResponse {
            ts: get_time() as i64,
            result: Some(result),
        }))
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "delete(YoriitoProducerServer)", level = "info", skip_all)]
    async fn delete(
        &self,
        request: tonic::Request<producer_viss::DeleteRequest>,
    ) -> std::result::Result<tonic::Response<producer_viss::DeleteResponse>, tonic::Status> {
        let inner: producer_viss::DeleteRequest = request.into_inner();
        let ret = self.interactor.delete(inner.path).await;
        let ret = ret.map_err(|error| common_viss::Error::from(&error));
        let result = match ret {
            Ok(()) => Some(producer_viss::delete_response::Result::Success(())),
            Err(error) => Some(producer_viss::delete_response::Result::Error(error)),
        };
        Ok(tonic::Response::new(producer_viss::DeleteResponse {
            ts: get_time() as i64,
            result,
        }))
    }

    type SubscribeTargetStream = Pin<
        Box<
            dyn tonic::codegen::tokio_stream::Stream<
                    Item = std::result::Result<
                        producer_viss::SubscribeTargetResponse,
                        tonic::Status,
                    >,
                > + Send
                + 'static,
        >,
    >;

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(
        name = "subscribe_target(YoriitoProducerServer)",
        level = "info",
        skip_all
    )]
    async fn subscribe_target(
        &self,
        request: tonic::Request<producer_viss::SubscribeTargetRequest>,
    ) -> std::result::Result<tonic::Response<Self::SubscribeTargetStream>, tonic::Status> {
        let inner = request.into_inner();
        let paths_filter = inner.filter.as_ref().and_then(|v| v.to_paths_filter());
        let misc_filter = inner.filter.as_ref().and_then(|v| v.to_misc_filter());
        let (sender, receiver) = mpsc::channel(10);
        let ret = self
            .interactor
            .subscribe(
                Arc::new(()),
                Arc::new(SubscriberYoriito {
                    sender: sender.clone(),
                }),
                subscription::SubscriptionCondition {
                    mode: subscription::Mode::Requested,
                    path: inner.path,
                    paths_filter,
                    misc_filter,
                },
            )
            .await;
        if let Err(e) = ret {
            error!("failed to subscribe: {e}");
            let error = common_viss::Error::from(&e);
            let message = producer_viss::subscribe_target_response::MessageType::Error(
                common_viss::SubscribeError { error: Some(error) },
            );
            let message_type = Some(message);
            let response = producer_viss::SubscribeTargetResponse {
                ts: get_time() as i64,
                message_type,
            };
            let _ = sender.send(Ok(response)).await;
        }

        type SubscribeRequestStream = Pin<
            Box<
                dyn tokio_stream::Stream<
                        Item = Result<producer_viss::SubscribeTargetResponse, Status>,
                    > + Send
                    + 'static,
            >,
        >;

        Ok(Response::new(
            Box::pin(ReceiverStream::new(receiver)) as SubscribeRequestStream
        ))
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "unsubscribe(YoriitoProducerServer)", level = "info", skip_all)]
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
            Ok(_) => Some(common_viss::unsubscribe_response::Result::Success(())),
            Err(error) => Some(common_viss::unsubscribe_response::Result::Error(error)),
        };
        let payload = common_viss::UnsubscribeResponse {
            ts: get_time() as i64,
            result,
        };
        Ok(Response::new(payload))
    }
}

#[derive(Debug)]
struct SubscriberYoriito {
    sender: Sender<Result<producer_viss::SubscribeTargetResponse, Status>>,
}

#[async_trait]
impl traits::Subscriber for SubscriberYoriito {
    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(
        name = "notify_subscribe_result(SubscriberYoriito)",
        level = "info",
        skip_all
    )]
    async fn notify_subscribe_result(
        &self,
        _object: Arc<dyn Any + Send + Sync>,
        subscription_id: String,
        ret: bool,
    ) -> result::KuraResult<()> {
        let message_type = if ret {
            let message = producer_viss::subscribe_target_response::MessageType::Success(
                common_viss::SubscribeSuccess { subscription_id },
            );
            Some(message)
        } else {
            let error = common_viss::Error {
                number: common_viss::ErrorCode::ErrorNotFound as i32,
                reason: common_viss::ErrorReason::ReasonUnspecified as i32,
                description: None,
            };
            let message = producer_viss::subscribe_target_response::MessageType::Error(
                common_viss::SubscribeError { error: Some(error) },
            );
            Some(message)
        };

        let response = producer_viss::SubscribeTargetResponse {
            ts: get_time() as i64,
            message_type,
        };

        self.sender.send(Ok(response)).await.to_kura_result(
            result::KuraErrorCode::Unspecified,
            result::KuraErrorReason::Unspecified,
            "send failed".to_string(),
        )
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "notify_signals(SubscriberYoriito)", level = "info", skip_all)]
    async fn notify_signals(
        &self,
        _object: Arc<dyn Any + Send + Sync>,
        subscription_id: String,
        signals: &[model::SignalStateView],
    ) -> result::KuraResult<()> {
        let data: result::KuraResult<Vec<common_viss::DataPackageTarget>> = signals
            .iter()
            .map(common_viss::DataPackageTarget::try_from)
            .collect();
        let data = Some(common_viss::DataPackagesTarget { data: data? });
        let event = producer_viss::SubscriptionTargetEvent {
            subscription_id,
            data,
        };
        let message = producer_viss::subscribe_target_response::MessageType::Event(event);
        let event = producer_viss::SubscribeTargetResponse {
            ts: get_time() as i64,
            message_type: Some(message),
        };

        self.sender.send(Ok(event)).await.to_kura_result(
            result::KuraErrorCode::Unspecified,
            result::KuraErrorReason::Unspecified,
            "send failed".to_string(),
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::entity::mem_database::MemDatabase;
    use crate::interactor::InteractorImpl;

    fn create_server() -> YoriitoProducerServer {
        let database = Arc::new(
            MemDatabase::create()
                .unwrap_or_else(|error| panic!("database creation should succeed: {error}")),
        );
        let interactor: Arc<dyn traits::Interactor + Send + Sync> = Arc::new(
            InteractorImpl::new(database)
                .unwrap_or_else(|error| panic!("interactor creation should succeed: {error}")),
        );
        YoriitoProducerServer::new("127.0.0.1:0", interactor)
            .unwrap_or_else(|error| panic!("server creation should succeed: {error}"))
    }

    fn assert_bad_request(result: Option<producer_viss::define_response::Result>) {
        let error = match result {
            Some(producer_viss::define_response::Result::Error(error)) => error,
            Some(producer_viss::define_response::Result::Success(())) => {
                panic!("define request should be rejected")
            }
            None => panic!("define response should contain a result"),
        };
        assert_eq!(error.number, common_viss::ErrorCode::ErrorBadRequest as i32);
        assert_eq!(
            error.reason,
            common_viss::ErrorReason::ReasonInvalidData as i32
        );
    }

    #[tokio::test]
    async fn rejects_define_without_entries() {
        let server = create_server();
        let request = producer_viss::DefineRequest { entries: None };

        let response = server
            .define(tonic::Request::new(request))
            .await
            .unwrap_or_else(|status| panic!("define should return a response: {status}"))
            .into_inner();

        assert_bad_request(response.result);
    }

    #[tokio::test]
    async fn rejects_define_with_invalid_struct_property_data_type() {
        let server = create_server();
        let property = common_viss::PropertyMetadataEntry {
            datatype: i32::MAX,
            ..Default::default()
        };
        let struct_metadata = common_viss::StructMetadataEntry {
            path: "Vehicle.FuzzStruct".to_string(),
            properties: vec![property],
            ..Default::default()
        };
        let request = producer_viss::DefineRequest {
            entries: Some(common_viss::MetadataEntries {
                metadata_entries: Vec::new(),
                struct_metadata_entries: vec![struct_metadata],
            }),
        };

        let response = server
            .define(tonic::Request::new(request))
            .await
            .unwrap_or_else(|status| panic!("define should return a response: {status}"))
            .into_inner();

        assert_bad_request(response.result);
    }
}
