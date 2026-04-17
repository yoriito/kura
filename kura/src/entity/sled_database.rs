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

use crate::interface::signal::{
    Config, DataPoint, KuraError, KuraErrorCode, KuraErrorReason, KuraResult,
    MapExternalToKuraResult, MapOptionToKuraError, Signal, Value,
};
use crate::interface::subscription::{GetCondition, Mode, SubscriptionCondition};
use crate::interface::{Database, Subscriber};
use bincode::config::standard;
use bincode::{decode_from_slice, encode_to_vec};
use futures;
use sled;
use std::any::Any;
use tracing::error;
use uuid::Uuid;

use std::sync::Arc;
use tonic::async_trait;

#[derive(Debug)]
struct SubscriptionRecord {
    object: Arc<dyn Any + Send + Sync>,
    subscription_id: String,
    subscriber: Arc<dyn Subscriber + Send + Sync>,
    #[allow(dead_code)]
    subscription_condition: SubscriptionCondition,
    previous_notified_value: Option<Value>,
    previous_uptime: u128,
    paths: Arc<tokio::sync::Mutex<Vec<String>>>,
}

impl SubscriptionRecord {
    pub fn new(
        subscription_id: String,
        object: Arc<dyn Any + Send + Sync>,
        subscriber: Arc<dyn Subscriber + Send + Sync>,
        subscription_condition: SubscriptionCondition,
        current_paths: &Vec<String>,
    ) -> KuraResult<Self> {
        let mut paths = [].to_vec();
        for current_path in current_paths {
            if subscription_condition.is_match_path(current_path) {
                paths.push(current_path.clone());
            }
        }

        Ok(Self {
            object: object.clone(),
            subscription_id: subscription_id.clone(),
            subscriber,
            subscription_condition,
            previous_notified_value: None,
            previous_uptime: 0,
            paths: Arc::new(tokio::sync::Mutex::new(paths)),
        })
    }

    pub async fn notify_subscribe_result(&self) -> KuraResult<()> {
        self.subscriber
            .notify_subscribe_result(self.object.clone(), self.subscription_id.clone(), true)
            .await
    }

    pub async fn notify_new_signal_insert(&self, key: &String) -> KuraResult<()> {
        if !self.subscription_condition.is_match_path(key) {
            return Ok(());
        }
        self.paths.lock().await.push(key.clone());

        Ok(())
    }

    pub async fn notify_set_event(&self, signal: &Signal, mode: Mode) -> KuraResult<()> {
        let mut is_target = false;
        for target_path in self.paths.lock().await.iter() {
            if target_path.clone() == signal.path.clone() {
                is_target = true;
                break;
            }
        }

        if !is_target {
            return Ok(());
        }

        if !self.subscription_condition.check_change_filter(
            &self.previous_notified_value,
            &signal.target_datapoint.value,
        ) {
            return Ok(());
        }

        if self.subscription_condition.mode != mode {
            return Ok(());
        }

        self.subscriber
            .notify_signals(
                self.object.clone(),
                self.subscription_id.clone(),
                [signal.clone()].as_ref(),
            )
            .await?;

        Ok(())
    }

    pub async fn notify_tick_event(
        &mut self,
        uptime: u128,
        signals: Vec<Signal>,
    ) -> KuraResult<()> {
        if !self
            .subscription_condition
            .check_tick_time_filter(self.previous_uptime, uptime)
        {
            return Ok(());
        }

        self.subscriber
            .notify_signals(self.object.clone(), self.subscription_id.clone(), &signals)
            .await?;
        self.previous_uptime = uptime;

        Ok(())
    }
}

pub struct SledDatabase {
    database: sled::Db,
    config: bincode::config::Configuration,
    subscription_records: tokio::sync::Mutex<Vec<SubscriptionRecord>>,
}

impl SledDatabase {
    #[allow(dead_code)]
    pub fn create() -> KuraResult<SledDatabase> {
        let database = sled::Config::new().temporary(true).open().to_kura_result(
            KuraErrorCode::Unspecified,
            KuraErrorReason::UnavailableData,
            "failed to create temporary database".to_string(),
        )?;
        Ok(SledDatabase {
            database,
            config: standard(),
            subscription_records: tokio::sync::Mutex::new(Vec::new()),
        })
    }

    #[allow(dead_code)]
    pub fn create_with_path<P: AsRef<std::path::Path>>(
        path: P,
    ) -> Result<SledDatabase, Box<dyn std::error::Error>> {
        Ok(SledDatabase {
            database: sled::Config::new().path(path).open()?,
            config: standard(),
            subscription_records: tokio::sync::Mutex::new(Vec::new()),
        })
    }

    fn get_inner_simple(&self, path: String) -> KuraResult<Signal> {
        let query_result = self
            .database
            .get(&path)
            .to_kura_result(
                KuraErrorCode::NotFound,
                KuraErrorReason::InvalidData,
                format!("{path} not found"),
            )?
            .to_kura_result(
                KuraErrorCode::NotFound,
                KuraErrorReason::InvalidData,
                format!("{path} not found"),
            )?;
        let (signal, _len) = decode_from_slice(&query_result, self.config).to_kura_result(
            KuraErrorCode::NotFound,
            KuraErrorReason::InvalidData,
            "decode error".to_string(),
        )?;
        assert_eq!(query_result.len(), _len);
        Ok(signal)
    }

    fn get_inner(&self, get_confition: GetCondition) -> KuraResult<Vec<Signal>> {
        let current_paths = self.get_all_keys();
        let target_paths = get_confition.get_matched_paths(&current_paths);

        if target_paths.is_empty() {
            let description = match get_confition.paths_filter {
                Some(filter) => {
                    format!("{} not found with filter: {:?}", get_confition.path, filter)
                }
                None => format!("{} not found", get_confition.path),
            };
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::NotFound,
                reason: KuraErrorReason::InvalidData,
                description: Some(description),
            });
        }

        let mut ret = [].to_vec();
        for target_path in target_paths {
            let query_result = self
                .database
                .get(&target_path)
                .to_kura_result(
                    KuraErrorCode::NotFound,
                    KuraErrorReason::InvalidData,
                    format!("{target_path} not found"),
                )?
                .to_kura_result(
                    KuraErrorCode::NotFound,
                    KuraErrorReason::InvalidData,
                    format!("{target_path} not found"),
                )?;
            let (signal, _len) = decode_from_slice(&query_result, self.config).to_kura_result(
                KuraErrorCode::NotFound,
                KuraErrorReason::InvalidData,
                "decode error".to_string(),
            )?;
            assert_eq!(query_result.len(), _len);
            ret.push(signal);
        }

        Ok(ret)
    }

    fn get_all_keys(&self) -> Vec<String> {
        self.database
            .iter()
            .keys()
            .filter_map(|res| match res {
                Ok(ivec) => Some(String::from_utf8_lossy(&ivec).to_string()),
                Err(e) => {
                    error!("Error reading key from sled: {}", e);
                    None
                }
            })
            .collect()
    }
}

impl std::fmt::Debug for SledDatabase {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("SledDatabase").finish()
    }
}

#[async_trait]
impl Database for SledDatabase {
    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "set_metadata(SledDatabaseset)", level = "info", skip_all)]
    async fn initialize_signal(&self, signal: &Signal) -> KuraResult<bool> {
        let encoded = encode_to_vec(signal, self.config).to_kura_result(
            KuraErrorCode::Unspecified,
            KuraErrorReason::Unspecified,
            "encode error".to_string(),
        )?;
        let is_contains_key = self
            .database
            .contains_key(signal.path.clone())
            .to_kura_result(
                KuraErrorCode::Unspecified,
                KuraErrorReason::Unspecified,
                "contains_key error".to_string(),
            )?;
        self.database
            .insert(signal.path.clone(), encoded)
            .to_kura_result(
                KuraErrorCode::Unspecified,
                KuraErrorReason::Unspecified,
                "insert error".to_string(),
            )?;
        if !is_contains_key {
            return Ok(true);
        }
        let subscription_records = self.subscription_records.lock().await;
        let futures = subscription_records
            .iter()
            .map(async |x| x.notify_new_signal_insert(&signal.path.clone()).await);
        futures::future::join_all(futures).await;
        Ok(true)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "update_metadata(SledDatabaseset)", level = "info", skip_all)]
    async fn update_metadata(&self, path: String, config: &Config) -> KuraResult<bool> {
        let get_condition = GetCondition {
            path: path.clone(),
            paths_filter: None,
        };
        let signals = self
            .get_current(get_condition.clone())
            .unwrap_or([].to_vec());
        if signals.len() != 1 {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::Unspecified,
                reason: KuraErrorReason::InvalidData,
                description: Some("too many data found".to_string()),
            });
        }
        let mut signal = signals[0].clone();
        signal.config = config.clone();
        let encoded = encode_to_vec(&signal, self.config).to_kura_result(
            KuraErrorCode::Unspecified,
            KuraErrorReason::Unspecified,
            "encoded failed".to_string(),
        )?;
        self.database
            .insert(signal.path.clone(), encoded)
            .to_kura_result(
                KuraErrorCode::Unspecified,
                KuraErrorReason::Unspecified,
                "insert failed".to_string(),
            )?;
        Ok(true)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "get(SledDatabaseset)", level = "info", skip_all)]
    fn get_current(&self, get_confition: GetCondition) -> KuraResult<Vec<Signal>> {
        self.get_inner(get_confition)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "get(SledDatabaseset)", level = "info", skip_all)]
    fn get_target(&self, get_confition: GetCondition) -> KuraResult<Vec<Signal>> {
        self.get_inner(get_confition)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "set(SledDatabaseset)", level = "info", skip_all)]
    async fn set_current(&self, path: String, datapoint: DataPoint) -> KuraResult<()> {
        let get_condition = GetCondition {
            path: path.clone(),
            paths_filter: None,
        };
        let signals = self.get_current(get_condition.clone())?;
        if signals.len() != 1 {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::NotFound,
                reason: KuraErrorReason::InvalidData,
                description: Some(format!("{path} not found")),
            });
        }
        let mut signal = signals[0].clone();
        // Maybe unnecessary but not sure
        //signal.request_id = request_id;
        signal.current_datapoint = datapoint;
        let encoded = encode_to_vec(&signal, self.config).to_kura_result(
            KuraErrorCode::Unspecified,
            KuraErrorReason::Unspecified,
            "encode failed".to_string(),
        )?;
        let contains_key = self
            .database
            .contains_key(signal.path.clone())
            .to_kura_result(
                KuraErrorCode::Unspecified,
                KuraErrorReason::Unspecified,
                "contains_key failed".to_string(),
            )?;
        if !contains_key {
            let subscription_records = self.subscription_records.lock().await;
            let futures = subscription_records
                .iter()
                .map(async |x| x.notify_new_signal_insert(&signal.path.clone()).await);
            futures::future::join_all(futures).await;
        }

        self.database
            .insert(signal.path.clone(), encoded)
            .to_kura_result(
                KuraErrorCode::Unspecified,
                KuraErrorReason::Unspecified,
                "insert failed".to_string(),
            )?;

        let signals = self.get_current(get_condition)?;
        if signals.len() != 1 {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::NotFound,
                reason: KuraErrorReason::InvalidData,
                description: Some(format!("{path} not found")),
            });
        }
        let signal = signals[0].clone();

        let mut invalid_subscriptions = [].to_vec();
        for (index, subscription_record) in
            self.subscription_records.lock().await.iter().enumerate()
        {
            let send_result = subscription_record
                .notify_set_event(&signal, Mode::Current)
                .await;
            if let Err(e) = send_result {
                error!("send failed: {}", e);
                invalid_subscriptions.push(index);
            }
        }

        for index in invalid_subscriptions {
            // drop subscription instance here and close sender.
            self.subscription_records.lock().await.remove(index);
        }

        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "set(SledDatabaseset)", level = "info", skip_all)]
    async fn set_target(&self, path: String, datapoint: DataPoint) -> KuraResult<()> {
        let get_condition = GetCondition {
            path: path.clone(),
            paths_filter: None,
        };
        let signals = self.get_current(get_condition.clone())?;
        if signals.len() != 1 {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::NotFound,
                reason: KuraErrorReason::InvalidData,
                description: Some(format!("{path} not found")),
            });
        }
        let mut signal = signals[0].clone();
        if signal.config.leaf_type != crate::interface::signal::LeafType::Actuator {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::BadRequest,
                reason: KuraErrorReason::BadRequest,
                description: Some(format!("{path} is not Actuator")),
            });
        }

        signal.target_datapoint = datapoint;
        let encoded = encode_to_vec(&signal, self.config).to_kura_result(
            KuraErrorCode::Unspecified,
            KuraErrorReason::Unspecified,
            "encode failed".to_string(),
        )?;
        if !self.database.contains_key(signal.path.clone()).unwrap() {
            for subscription_record in self.subscription_records.lock().await.iter() {
                subscription_record.notify_new_signal_insert(&path).await?;
            }
        }

        self.database
            .insert(signal.path.clone(), encoded)
            .to_kura_result(
                KuraErrorCode::Unspecified,
                KuraErrorReason::Unspecified,
                "insert failed".to_string(),
            )?;
        let signals = self.get_current(get_condition)?;
        if signals.len() != 1 {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::NotFound,
                reason: KuraErrorReason::InvalidData,
                description: Some(format!("{path} not found")),
            });
        }
        let signal = signals[0].clone();

        let mut invalid_subscriptions = [].to_vec();
        for (index, subscription_record) in
            self.subscription_records.lock().await.iter().enumerate()
        {
            let send_result = subscription_record
                .notify_set_event(&signal, Mode::Target)
                .await;
            if let Err(e) = send_result {
                error!("send failed: {}", e);
                invalid_subscriptions.push(index);
            }
        }

        for index in invalid_subscriptions {
            // drop subscription instance here and close sender.
            self.subscription_records.lock().await.remove(index);
        }

        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace", skip(subscriber))]
    #[tracing::instrument(name = "subscribe(SledDatabaseset)", level = "info", skip_all)]
    async fn subscribe(
        &self,
        object: Arc<dyn Any + Send + Sync>,
        subscriber: Arc<dyn Subscriber + Send + Sync>,
        subscription_condition: SubscriptionCondition,
    ) -> KuraResult<()> {
        let subscription_id = Uuid::new_v4().to_string();
        let keys = self.get_all_keys();
        let record = SubscriptionRecord::new(
            subscription_id.clone(),
            object,
            subscriber,
            subscription_condition,
            &keys,
        )?;
        let mut subscription_record = self.subscription_records.lock().await;
        record.notify_subscribe_result().await?;
        subscription_record.push(record);

        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "unsubscribe(SledDatabaseset)", level = "info", skip_all)]
    async fn unsubscribe(&self, subscribe_id: String) -> KuraResult<()> {
        let mut subscription_record = self.subscription_records.lock().await;
        // drop sender object, and let client know the channel is closed
        let _ = subscription_record.pop_if(|x| x.subscription_id == subscribe_id);
        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "delete(SledDatabaseset)", level = "info", skip_all)]
    async fn delete(&self, path: Vec<String>) -> KuraResult<()> {
        for target_path in path {
            let _ = self.database.remove(target_path).to_kura_result(
                KuraErrorCode::Unspecified,
                KuraErrorReason::Unspecified,
                "delete failed".to_string(),
            )?;
        }
        Ok(())
    }

    async fn tick(&self, uptime: u128) -> KuraResult<()> {
        let mut invalid_subscriptions = [].to_vec();
        for (index, subscription_record) in self
            .subscription_records
            .lock()
            .await
            .iter_mut()
            .enumerate()
        {
            let signals: KuraResult<Vec<Signal>> = subscription_record
                .paths
                .lock()
                .await
                .iter()
                .map(|x| self.get_inner_simple(x.to_string()))
                .collect();
            let signals = signals?;
            let tick_result = subscription_record.notify_tick_event(uptime, signals).await;
            if let Err(e) = tick_result {
                error!("tick failed: {}", e);
                invalid_subscriptions.push(index);
            }
        }

        for index in invalid_subscriptions {
            // drop subscription instance here and close sender.
            self.subscription_records.lock().await.remove(index);
        }
        Ok(())
    }
}
