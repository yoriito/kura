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
    Config, DataPoint, KuraError, KuraErrorCode, KuraErrorReason, KuraResult, MapOptionToKuraError,
    Signal, Value,
};
use crate::interface::subscription::{GetCondition, Mode, SubscriptionCondition};
use crate::interface::{Database, Subscriber};
use bincode::config::standard;
use std::any::Any;
use tracing::error;
use uuid::Uuid;

use dashmap::DashMap;
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
        current_paths: &[String],
    ) -> KuraResult<Self> {
        let paths = subscription_condition.get_matched_paths(current_paths);

        if paths.is_empty() {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::NotFound,
                reason: KuraErrorReason::InvalidData,
                description: Some("path not found".to_string()),
            });
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

pub struct MemDatabase {
    database: DashMap<String, Signal>,
    #[allow(dead_code)]
    config: bincode::config::Configuration,
    subscription_records: tokio::sync::Mutex<Vec<SubscriptionRecord>>,
}

impl MemDatabase {
    pub fn create() -> KuraResult<MemDatabase> {
        Ok(MemDatabase {
            database: DashMap::new(),
            config: standard(),
            subscription_records: tokio::sync::Mutex::new(Vec::new()),
        })
    }

    fn get_inner_simple(&self, path: String) -> KuraResult<Signal> {
        let ret = self
            .database
            .get(&path)
            .to_kura_result(
                KuraErrorCode::NotFound,
                KuraErrorReason::InvalidData,
                format!("{path} not found"),
            )?
            .clone();
        Ok(ret)
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
            let signal = self
                .database
                .get(&target_path)
                .to_kura_result(
                    KuraErrorCode::NotFound,
                    KuraErrorReason::InvalidData,
                    format!("{target_path} not found"),
                )?
                .clone();
            ret.push(signal);
        }

        Ok(ret)
    }

    fn get_all_keys(&self) -> Vec<String> {
        self.database
            .iter()
            .map(|record| record.key().clone())
            .collect()
    }
}

impl std::fmt::Debug for MemDatabase {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("MemDatabaseset").finish()
    }
}

#[async_trait]
impl Database for MemDatabase {
    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "set_metadata(MemDatabaseset)", level = "info", skip_all)]
    async fn initialize_signal(&self, signal: &Signal) -> KuraResult<bool> {
        if !self.database.contains_key(&signal.path) {
            for subscription_record in self.subscription_records.lock().await.iter() {
                subscription_record
                    .notify_new_signal_insert(&signal.path)
                    .await?;
            }
        }

        self.database.insert(signal.path.clone(), signal.clone());
        Ok(true)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "update_metadata(MemDatabaseset)", level = "info", skip_all)]
    async fn update_metadata(&self, path: String, config: &Config) -> KuraResult<bool> {
        let get_condition = GetCondition {
            path: path.clone(),
            paths_filter: None,
        };
        let signals = self
            .get_current(get_condition.clone())
            .unwrap_or([].to_vec());
        if signals.len() != 1 {
            let dp = DataPoint {
                stored_timestamp: None,
                produced_timestamp: None,
                value: Value::Nan,
                is_available_sensor: None,
                is_available_actuator: None,
            };
            let signal = Signal {
                path: path.clone(),
                current_datapoint: dp.clone(),
                target_datapoint: dp,
                config: config.clone(),
            };
            let _ = self.database.insert(path.clone(), signal);
            return Ok(true);
        }
        let mut signal = signals[0].clone();
        signal.config = config.clone();
        let _ = self.database.insert(signal.path.clone(), signal);
        Ok(true)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "get(MemDatabaseset)", level = "info", skip_all)]
    fn get_current(&self, get_confition: GetCondition) -> KuraResult<Vec<Signal>> {
        self.get_inner(get_confition)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "get(MemDatabaseset)", level = "info", skip_all)]
    fn get_target(&self, get_confition: GetCondition) -> KuraResult<Vec<Signal>> {
        self.get_inner(get_confition)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "set(MemDatabaseset)", level = "info", skip_all)]
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
        signal.current_datapoint = datapoint.clone();
        if !self.database.contains_key(&signal.path) {
            for subscription_record in self.subscription_records.lock().await.iter() {
                subscription_record.notify_new_signal_insert(&path).await?;
            }
        }

        self.database.insert(signal.path.clone(), signal);

        let signals = self.get_current(get_condition)?;
        if signals.len() != 1 {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::NotFound,
                reason: KuraErrorReason::InvalidData,
                description: Some(format!("{path} not found")),
            });
        }
        let signal = signals[0].clone();
        if signal.config.leaf_type == crate::interface::signal::LeafType::Branch {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::BadRequest,
                reason: KuraErrorReason::BadRequest,
                description: Some(format!("{path} is not valid node type")),
            });
        }

        if !signal.config.data_type.is_match(&datapoint.value) {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::BadRequest,
                reason: KuraErrorReason::BadRequest,
                description: Some(format!(
                    "{} is set, but {} is {}",
                    datapoint.value, path, signal.config.data_type
                )),
            });
        }

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

        invalid_subscriptions.sort_unstable();
        invalid_subscriptions.reverse();
        for index in invalid_subscriptions {
            // drop subscription instance here and close sender.
            self.subscription_records.lock().await.remove(index);
        }

        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "set(MemDatabaseset)", level = "info", skip_all)]
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

        if !signal.config.data_type.is_match(&datapoint.value) {
            return KuraResult::Err(KuraError {
                number: KuraErrorCode::BadRequest,
                reason: KuraErrorReason::BadRequest,
                description: Some(format!(
                    "{} is set, but {} is {}",
                    datapoint.value, path, signal.config.data_type
                )),
            });
        }

        signal.target_datapoint = datapoint;
        if !self.database.contains_key(&signal.path) {
            for subscription_record in self.subscription_records.lock().await.iter() {
                subscription_record.notify_new_signal_insert(&path).await?;
            }
        }

        self.database.insert(signal.path.clone(), signal);
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
    #[tracing::instrument(name = "subscribe(MemDatabaseset)", level = "info", skip_all)]
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
    #[tracing::instrument(name = "unsubscribe(MemDatabaseset)", level = "info", skip_all)]
    async fn unsubscribe(&self, subscribe_id: String) -> KuraResult<()> {
        let mut subscription_record = self.subscription_records.lock().await;
        // drop sender object, and let client know the channel is closed
        let _ = subscription_record.pop_if(|x| x.subscription_id == subscribe_id);
        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "delete(MemDatabaseset)", level = "info", skip_all)]
    async fn delete(&self, path: Vec<String>) -> KuraResult<()> {
        let mut exist_paths = self.get_all_keys();
        exist_paths.sort_by_key(|key| key.len());
        let target_paths_map: DashMap<&String, Vec<String>> = DashMap::new();
        for cur_path in &path {
            let mut target_paths: Vec<String> = [].to_vec();
            let cur_path_as_branch = cur_path.clone() + ".";
            for exist_path in &exist_paths {
                if exist_path.starts_with(&cur_path_as_branch) {
                    target_paths.push(exist_path.clone());
                }
            }

            if !target_paths.is_empty() {
                target_paths_map.insert(cur_path, target_paths);
                continue;
            }

            for exist_path in &exist_paths {
                if exist_path.starts_with(cur_path) {
                    target_paths.push(exist_path.clone());
                    break;
                }
            }

            if target_paths.is_empty() {
                return KuraResult::Err(KuraError {
                    number: KuraErrorCode::NotFound,
                    reason: KuraErrorReason::InvalidData,
                    description: Some(format!("{cur_path} not found")),
                });
            }

            target_paths_map.insert(cur_path, target_paths);
        }

        for entry in target_paths_map.iter() {
            let (_, values) = entry.pair();
            for value in values {
                self.database.remove(value);
            }
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
