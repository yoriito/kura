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

#![no_main]

use std::sync::{Arc, OnceLock};
use std::time::Duration;

use libfuzzer_sys::arbitrary;
use libfuzzer_sys::arbitrary::Arbitrary;
use libfuzzer_sys::fuzz_target;
use tokio::runtime::Runtime;
use tokio_util::sync::CancellationToken;

use kura::controller::yoriito::producer_server::YoriitoProducerServer;
use kura::entity::mem_database::MemDatabase;
use kura::interface::model::{LeafType, SignalMetadata, ValueType};
use kura::interface::traits::{Database, Interactor};
use kura::interactor::InteractorImpl;

#[allow(clippy::enum_variant_names)]
#[allow(dead_code)]
mod viss {
    tonic::include_proto!("yoriito.viss.v1");
    pub mod producer {
        tonic::include_proto!("yoriito.viss.v1.producer");
    }
}

static RT: OnceLock<Runtime> = OnceLock::new();
static CHANNEL: OnceLock<tonic::transport::Channel> = OnceLock::new();
static DB: OnceLock<Arc<MemDatabase>> = OnceLock::new();

#[derive(Debug, Arbitrary)]
enum FuzzValue {
    StringVal(String),
    BoolVal(bool),
    Int8Val(i32),
    Int32Val(i32),
    Int64Val(i64),
    Uint32Val(u32),
    Uint64Val(u64),
    FloatVal(f32),
    DoubleVal(f64),
}

impl From<FuzzValue> for viss::value_type::ValueType {
    fn from(v: FuzzValue) -> Self {
        match v {
            FuzzValue::StringVal(s) => Self::String(s),
            FuzzValue::BoolVal(b) => Self::Bool(b),
            FuzzValue::Int8Val(i) => Self::Int8(i),
            FuzzValue::Int32Val(i) => Self::Int32(i),
            FuzzValue::Int64Val(i) => Self::Int64(i),
            FuzzValue::Uint32Val(u) => Self::Uint32(u),
            FuzzValue::Uint64Val(u) => Self::Uint64(u),
            FuzzValue::FloatVal(f) => Self::Float(f),
            FuzzValue::DoubleVal(d) => Self::Double(d),
        }
    }
}

#[derive(Debug, Arbitrary)]
struct FuzzGetTargetRequest {
    path: String,
    filter_paths: Option<Vec<String>>,
}

impl From<FuzzGetTargetRequest> for viss::producer::GetTargetRequest {
    fn from(input: FuzzGetTargetRequest) -> Self {
        Self {
            path: input.path,
            filter: input.filter_paths.map(|paths| viss::GetFilter {
                parameter_variant: Some(viss::get_filter::ParameterVariant::PathsParameter(
                    viss::PathsFilter { paths },
                )),
            }),
        }
    }
}

#[derive(Debug, Arbitrary)]
struct FuzzDataPointCurrent {
    value: Option<FuzzValue>,
    stored_timestamp: i64,
    produced_timestamp: Option<i64>,
    is_available_sensor: Option<bool>,
    is_available_actuator: Option<bool>,
}

#[derive(Debug, Arbitrary)]
struct FuzzSetCurrentRequest {
    path: String,
    dp: Vec<FuzzDataPointCurrent>,
}

impl From<FuzzSetCurrentRequest> for viss::producer::SetCurrentRequest {
    fn from(input: FuzzSetCurrentRequest) -> Self {
        Self {
            data: Some(viss::DataPackagesCurrent {
                data: vec![viss::DataPackageCurrent {
                    path: input.path,
                    dp: input
                        .dp
                        .into_iter()
                        .map(|d| viss::DataPointCurrent {
                            stored_timestamp: d.stored_timestamp,
                            value: Some(viss::ValueType {
                                value_type: d.value.map(viss::value_type::ValueType::from),
                            }),
                            produced_timestamp: d.produced_timestamp,
                            is_available_sensor: d.is_available_sensor,
                            is_available_actuator: d.is_available_actuator,
                        })
                        .collect(),
                }],
            }),
        }
    }
}

#[derive(Debug, Arbitrary)]
enum FuzzInput {
    GetTarget(FuzzGetTargetRequest),
    SetCurrent(FuzzSetCurrentRequest),
}

async fn start_test_server() -> u16 {
    let listener = tokio::net::TcpListener::bind("127.0.0.1:0").await.unwrap();
    let port = listener.local_addr().unwrap().port();
    drop(listener);

    let database = Arc::new(
        MemDatabase::create().unwrap_or_else(|_| panic!("MemDatabase::create failed")),
    );
    DB.get_or_init(|| database.clone());
    let interactor: Arc<dyn Interactor + Send + Sync> = Arc::new(
        InteractorImpl::new(database).unwrap_or_else(|_| panic!("InteractorImpl::new failed")),
    );

    let server = YoriitoProducerServer::new(&format!("127.0.0.1:{port}"), interactor)
        .unwrap_or_else(|_| panic!("YoriitoProducerServer::new failed"));
    let cancel = CancellationToken::new();
    let cancel_server = cancel.clone();
    tokio::spawn(async move {
        let _ = server.start(cancel_server).await;
    });

    tokio::time::sleep(Duration::from_millis(100)).await;
    port
}

fn get_channel(rt: &Runtime) -> tonic::transport::Channel {
    CHANNEL
        .get_or_init(|| {
            rt.block_on(async {
                let port = start_test_server().await;
                tonic::transport::Channel::from_shared(format!("http://127.0.0.1:{port}"))
                    .unwrap()
                    .connect()
                    .await
                    .unwrap()
            })
        })
        .clone()
}

fuzz_target!(|input: FuzzInput| {
    let rt = RT.get_or_init(|| Runtime::new().unwrap());
    let channel = get_channel(rt);
    let mut client = viss::producer::viss_client::VissClient::new(channel);

    let path = match &input {
        FuzzInput::GetTarget(req) => req.path.clone(),
        FuzzInput::SetCurrent(req) => req.path.clone(),
    };

    if let Some(db) = DB.get() {
        let metadata = SignalMetadata {
            leaf_type: LeafType::Actuator,
            data_type: ValueType::TypeDouble,
            deprecation: None,
            unit: None,
            min: None,
            max: None,
            description: None,
            comment: None,
            allowd: None,
            default: None,
            array_size: None,
            pattern: None,
            user_defined_items: None,
        };
        let _ = rt.block_on(db.initialize_signal(path, metadata));
    }

    match input {
        FuzzInput::GetTarget(req) => {
            let _ =
                rt.block_on(client.get_target(viss::producer::GetTargetRequest::from(req)));
        }
        FuzzInput::SetCurrent(req) => {
            let _ =
                rt.block_on(client.set_current(viss::producer::SetCurrentRequest::from(req)));
        }
    }
});
