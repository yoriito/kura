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

use crate::interface::{model, result, result::MapOptionToKuraError, subscription, traits};
use bincode::config::standard;
use std::any::Any;
use std::collections::HashMap;
use tracing::error;
use uuid::Uuid;

use dashmap::DashMap;
use std::sync::Arc;
use tonic::async_trait;

#[derive(Debug)]
struct SubscriptionRecord {
    object: Arc<dyn Any + Send + Sync>,
    subscription_id: String,
    subscriber: Arc<dyn traits::Subscriber + Send + Sync>,
    #[allow(dead_code)]
    subscription_condition: subscription::SubscriptionCondition,
    previous_notified_state: Option<model::SignalState>,
    previous_uptime: u128,
    paths: Arc<tokio::sync::Mutex<Vec<String>>>,
}

impl SubscriptionRecord {
    pub fn new(
        subscription_id: String,
        object: Arc<dyn Any + Send + Sync>,
        subscriber: Arc<dyn traits::Subscriber + Send + Sync>,
        subscription_condition: subscription::SubscriptionCondition,
        current_paths: &[String],
    ) -> result::KuraResult<Self> {
        let paths = subscription_condition.get_matched_paths(current_paths)?;

        if paths.is_empty() {
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::NotFound,
                reason: result::KuraErrorReason::InvalidData,
                description: Some("path not found".to_string()),
            });
        }

        Ok(Self {
            object: object.clone(),
            subscription_id: subscription_id.clone(),
            subscriber,
            subscription_condition,
            previous_notified_state: None,
            previous_uptime: 0,
            paths: Arc::new(tokio::sync::Mutex::new(paths)),
        })
    }

    pub async fn notify_subscribe_result(&self) -> result::KuraResult<()> {
        self.subscriber
            .notify_subscribe_result(self.object.clone(), self.subscription_id.clone(), true)
            .await
    }

    pub async fn notify_new_signal_insert(&self, key: &String) -> result::KuraResult<()> {
        if !self.subscription_condition.is_match_path(key)? {
            return Ok(());
        }
        self.paths.lock().await.push(key.clone());

        Ok(())
    }

    pub async fn notify_set_event(
        &self,
        state_view: &model::SignalStateView,
        mode: subscription::Mode,
    ) -> result::KuraResult<()> {
        let mut is_target = false;
        for target_path in self.paths.lock().await.iter() {
            if target_path.clone() == state_view.path.clone() {
                is_target = true;
                break;
            }
        }

        if !is_target {
            return Ok(());
        }

        if !self
            .subscription_condition
            .check_change_filter(&self.previous_notified_state, &state_view.state)
        {
            return Ok(());
        }

        if self.subscription_condition.mode != mode {
            return Ok(());
        }

        self.subscriber
            .notify_signals(
                self.object.clone(),
                self.subscription_id.clone(),
                [state_view.clone()].as_ref(),
            )
            .await?;

        Ok(())
    }

    pub async fn notify_tick_event(
        &mut self,
        uptime: u128,
        state_views: Vec<model::SignalStateView>,
    ) -> result::KuraResult<()> {
        if !self
            .subscription_condition
            .check_tick_time_filter(self.previous_uptime, uptime)
        {
            return Ok(());
        }

        self.subscriber
            .notify_signals(
                self.object.clone(),
                self.subscription_id.clone(),
                &state_views,
            )
            .await?;
        self.previous_uptime = uptime;

        Ok(())
    }
}

pub struct MemDatabase {
    actual_state_database: DashMap<String, model::SignalState>,
    requested_state_database: DashMap<String, model::SignalState>,
    metadata_database: DashMap<String, model::SignalMetadata>,
    vss_user_defined_type_database: DashMap<String, model::VSSUserDefinedType>,
    #[allow(dead_code)]
    config: bincode::config::Configuration,
    subscription_records: tokio::sync::Mutex<HashMap<String, SubscriptionRecord>>,
}

impl MemDatabase {
    pub fn create() -> result::KuraResult<MemDatabase> {
        Ok(MemDatabase {
            actual_state_database: DashMap::new(),
            requested_state_database: DashMap::new(),
            metadata_database: DashMap::new(),
            vss_user_defined_type_database: DashMap::new(),
            config: standard(),
            subscription_records: tokio::sync::Mutex::new(HashMap::new()),
        })
    }

    fn get_actual_state_view(&self, path: String) -> result::KuraResult<model::SignalStateView> {
        let state = self
            .actual_state_database
            .get(&path)
            .to_kura_result(
                result::KuraErrorCode::NotFound,
                result::KuraErrorReason::InvalidData,
                format!("{path} not found"),
            )?
            .clone();
        Ok(model::SignalStateView { path, state })
    }

    fn get_requested_state_view_by_condition(
        &self,
        get_confition: subscription::GetCondition,
    ) -> result::KuraResult<Vec<model::SignalStateView>> {
        let current_paths = self.get_all_keys();
        let target_paths = get_confition.get_matched_paths(&current_paths)?;
        if target_paths.is_empty() {
            let description = match get_confition.paths_filter {
                Some(filter) => {
                    format!("{} not found with filter: {:?}", get_confition.path, filter)
                }
                None => format!("{} not found", get_confition.path),
            };
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::NotFound,
                reason: result::KuraErrorReason::InvalidData,
                description: Some(description),
            });
        }

        let mut ret = [].to_vec();
        for target_path in target_paths {
            let state = self
                .requested_state_database
                .get(&target_path)
                .to_kura_result(
                    result::KuraErrorCode::NotFound,
                    result::KuraErrorReason::InvalidData,
                    format!("{target_path} not found"),
                )?
                .clone();
            ret.push(model::SignalStateView {
                path: target_path,
                state,
            });
        }

        Ok(ret)
    }

    fn get_actual_state_view_by_condition(
        &self,
        get_confition: subscription::GetCondition,
    ) -> result::KuraResult<Vec<model::SignalStateView>> {
        let current_paths = self.get_all_keys();
        let target_paths = get_confition.get_matched_paths(&current_paths)?;
        if target_paths.is_empty() {
            let description = match get_confition.paths_filter {
                Some(filter) => {
                    format!("{} not found with filter: {:?}", get_confition.path, filter)
                }
                None => format!("{} not found", get_confition.path),
            };
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::NotFound,
                reason: result::KuraErrorReason::InvalidData,
                description: Some(description),
            });
        }

        let mut ret = [].to_vec();
        for target_path in target_paths {
            let state = self
                .actual_state_database
                .get(&target_path)
                .to_kura_result(
                    result::KuraErrorCode::NotFound,
                    result::KuraErrorReason::InvalidData,
                    format!("{target_path} not found"),
                )?
                .clone();
            ret.push(model::SignalStateView {
                path: target_path,
                state,
            });
        }

        Ok(ret)
    }

    fn get_metadata_view_by_condition(
        &self,
        get_confition: subscription::GetCondition,
    ) -> result::KuraResult<Vec<model::SignalMetadataView>> {
        let current_paths = self.get_all_keys();
        let target_paths = get_confition.get_matched_paths(&current_paths)?;
        if target_paths.is_empty() {
            let description = match get_confition.paths_filter {
                Some(filter) => {
                    format!("{} not found with filter: {:?}", get_confition.path, filter)
                }
                None => format!("{} not found", get_confition.path),
            };
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::NotFound,
                reason: result::KuraErrorReason::InvalidData,
                description: Some(description),
            });
        }

        let mut ret = [].to_vec();
        for target_path in target_paths {
            let metadata = self
                .metadata_database
                .get(&target_path)
                .to_kura_result(
                    result::KuraErrorCode::NotFound,
                    result::KuraErrorReason::InvalidData,
                    format!("{target_path} not found"),
                )?
                .clone();
            ret.push(model::SignalMetadataView {
                path: target_path,
                metadata,
            });
        }

        Ok(ret)
    }

    fn create_default_state(&self, metadata: &model::SignalMetadata) -> model::SignalState {
        let value = if let Some(value) = &metadata.default {
            value.clone()
        } else {
            model::Value::Nan
        };
        model::SignalState {
            stored_timestamp: None,
            produced_timestamp: None,
            value,
            is_available_sensor: None,
            is_available_actuator: None,
        }
    }

    fn get_all_keys(&self) -> Vec<String> {
        self.metadata_database
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
impl traits::Database for MemDatabase {
    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "initialize_signal(MemDatabaseset)", level = "info", skip_all)]
    async fn initialize_signal(
        &self,
        path: String,
        signal: model::SignalMetadata,
    ) -> result::KuraResult<bool> {
        if !self.metadata_database.contains_key(&path) {
            for subscription_record in self.subscription_records.lock().await.values() {
                subscription_record.notify_new_signal_insert(&path).await?;
            }
        }
        let default_state = self.create_default_state(&signal);
        self.actual_state_database
            .insert(path.clone(), default_state.clone());
        self.requested_state_database
            .insert(path.clone(), default_state);
        self.metadata_database.insert(path, signal);
        Ok(true)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(
        name = "initialize_vss_user_defined_types(MemDatabaseset)",
        level = "info",
        skip_all
    )]
    async fn initialize_vss_user_defined_types(
        &self,
        vss_user_defined_types: Vec<model::VSSUserDefinedType>,
    ) -> result::KuraResult<bool> {
        for vss_user_defined_type in vss_user_defined_types {
            self.vss_user_defined_type_database
                .insert(vss_user_defined_type.name.clone(), vss_user_defined_type);
        }
        Ok(true)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "update_metadata(MemDatabaseset)", level = "info", skip_all)]
    async fn update_metadata(
        &self,
        path: String,
        config: &model::SignalMetadata,
    ) -> result::KuraResult<bool> {
        let get_condition = subscription::GetCondition {
            path: path.clone(),
            paths_filter: None,
        };

        let metadatas = match self.get_metadata_view_by_condition(get_condition) {
            Ok(metadatas) => metadatas,
            // in case of NotFound, will create new metadata.
            Err(error) if error.number == result::KuraErrorCode::NotFound => Vec::new(),
            Err(error) => return Err(error),
        };
        if metadatas.len() != 1 {
            let default_state = self.create_default_state(config);
            self.actual_state_database
                .insert(path.clone(), default_state.clone());
            self.requested_state_database
                .insert(path.clone(), default_state);
            let _ = self.metadata_database.insert(path.clone(), config.clone());
            return Ok(true);
        }

        let metadata_view = metadatas[0].clone();
        let _ = self
            .metadata_database
            .insert(metadata_view.path.clone(), config.clone());
        Ok(true)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "get_actual_state(MemDatabaseset)", level = "info", skip_all)]
    fn get_actual_state(
        &self,
        get_confition: subscription::GetCondition,
    ) -> result::KuraResult<Vec<model::SignalStateView>> {
        self.get_actual_state_view_by_condition(get_confition)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "get_requested_state(MemDatabaseset)", level = "info", skip_all)]
    fn get_requested_state(
        &self,
        get_confition: subscription::GetCondition,
    ) -> result::KuraResult<Vec<model::SignalStateView>> {
        self.get_requested_state_view_by_condition(get_confition)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "get_metadata(MemDatabaseset)", level = "info", skip_all)]
    fn get_metadata(
        &self,
        get_condition: subscription::GetCondition,
    ) -> result::KuraResult<Vec<model::SignalMetadataView>> {
        let current_paths = self.get_all_keys();
        let target_paths = get_condition.get_matched_paths(&current_paths)?;
        if target_paths.is_empty() {
            let description = match get_condition.paths_filter {
                Some(filter) => {
                    format!("{} not found with filter: {:?}", get_condition.path, filter)
                }
                None => format!("{} not found", get_condition.path),
            };
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::NotFound,
                reason: result::KuraErrorReason::InvalidData,
                description: Some(description),
            });
        }

        let mut ret = [].to_vec();
        for target_path in target_paths {
            let metadata = self
                .metadata_database
                .get(&target_path)
                .to_kura_result(
                    result::KuraErrorCode::NotFound,
                    result::KuraErrorReason::InvalidData,
                    format!("{target_path} not found"),
                )?
                .clone();
            ret.push(model::SignalMetadataView {
                path: target_path,
                metadata,
            });
        }

        Ok(ret)
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(
        name = "get_all_vss_user_defined_types(MemDatabaseset)",
        level = "info",
        skip_all
    )]
    fn get_all_vss_user_defined_types(
        &self,
    ) -> result::KuraResult<HashMap<String, model::VSSUserDefinedType>> {
        Ok(self
            .vss_user_defined_type_database
            .iter()
            .map(|entry| (entry.key().clone(), entry.value().clone()))
            .collect())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "set_actual_state(MemDatabaseset)", level = "info", skip_all)]
    async fn set_actual_state(
        &self,
        path: String,
        state: model::SignalState,
    ) -> result::KuraResult<()> {
        let get_condition = subscription::GetCondition {
            path: path.clone(),
            paths_filter: None,
        };

        // check compatibility of datatype
        let metadatas = self.get_metadata_view_by_condition(get_condition.clone())?;
        if metadatas.len() != 1 {
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::NotFound,
                reason: result::KuraErrorReason::InvalidData,
                description: Some(format!("{path} not found")),
            });
        }
        let metadata_view = metadatas[0].clone();
        if !metadata_view.metadata.is_match(&state) {
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::BadRequest,
                reason: result::KuraErrorReason::BadRequest,
                description: Some("incompatible datatype".to_string()),
            });
        }

        // update
        let states = self.get_actual_state_view_by_condition(get_condition.clone())?;
        if states.len() != 1 {
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::NotFound,
                reason: result::KuraErrorReason::InvalidData,
                description: Some(format!("{path} not found")),
            });
        }
        let mut state_view = states[0].clone();
        if !self.metadata_database.contains_key(&state_view.path) {
            for subscription_record in self.subscription_records.lock().await.values() {
                subscription_record.notify_new_signal_insert(&path).await?;
            }
        }
        self.actual_state_database
            .insert(state_view.path.clone(), state.clone());

        // notify subscription
        {
            state_view.state = state;
            let mut invalid_subscription_ids = [].to_vec();
            let mut subscription_records = self.subscription_records.lock().await;
            for (subscription_id, record) in subscription_records.iter() {
                let send_result = record
                    .notify_set_event(&state_view, subscription::Mode::Actual)
                    .await;
                if let Err(e) = send_result {
                    error!("send failed: {}", e);
                    invalid_subscription_ids.push(subscription_id.clone());
                }
            }

            for subscription_id in invalid_subscription_ids {
                // drop subscription instance here and close sender.
                let _ = subscription_records.remove(&subscription_id);
            }
        } // unlock subscription_records

        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "set_requested_state(MemDatabaseset)", level = "info", skip_all)]
    async fn set_requested_state(
        &self,
        path: String,
        state: model::SignalState,
    ) -> result::KuraResult<()> {
        let get_condition = subscription::GetCondition {
            path: path.clone(),
            paths_filter: None,
        };

        // check compatibility of datatype
        let metadatas = self.get_metadata_view_by_condition(get_condition.clone())?;
        if metadatas.len() != 1 {
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::NotFound,
                reason: result::KuraErrorReason::InvalidData,
                description: Some(format!("{path} not found")),
            });
        }
        let metadata_view = metadatas[0].clone();
        if metadata_view.metadata.leaf_type != model::LeafType::Actuator {
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::BadRequest,
                reason: result::KuraErrorReason::BadRequest,
                description: Some(format!("{path} is not Actuator")),
            });
        }
        if !metadata_view.metadata.is_match(&state) {
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::BadRequest,
                reason: result::KuraErrorReason::BadRequest,
                description: Some("incompatible datatype".to_string()),
            });
        }

        // check availability as actuator
        // None means the platform has not reported availability yet, which is not a denial.
        let is_available_actuator = self
            .actual_state_database
            .get(&metadata_view.path)
            .and_then(|actual| actual.is_available_actuator);
        if is_available_actuator == Some(false) {
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::ServiceUnavailable,
                reason: result::KuraErrorReason::UnavailableData,
                description: Some(format!("{path} is not available as actuator")),
            });
        }

        // update
        let states = self.get_requested_state_view_by_condition(get_condition.clone())?;
        if states.len() != 1 {
            return result::KuraResult::Err(result::KuraError {
                number: result::KuraErrorCode::NotFound,
                reason: result::KuraErrorReason::InvalidData,
                description: Some(format!("{path} not found")),
            });
        }
        let mut state_view = states[0].clone();
        if !self.metadata_database.contains_key(&state_view.path) {
            for subscription_record in self.subscription_records.lock().await.values() {
                subscription_record.notify_new_signal_insert(&path).await?;
            }
        }
        self.requested_state_database
            .insert(state_view.path.clone(), state.clone());

        // notify subscription
        state_view.state = state;

        {
            let mut subscription_records = self.subscription_records.lock().await;
            let mut invalid_subscription_ids = [].to_vec();
            for (subscription_id, record) in subscription_records.iter() {
                let send_result = record
                    .notify_set_event(&state_view, subscription::Mode::Requested)
                    .await;
                if let Err(e) = send_result {
                    error!("send failed: {}", e);
                    invalid_subscription_ids.push(subscription_id.clone());
                }
            }

            for subscription_id in invalid_subscription_ids {
                // drop subscription instance here and close sender.
                let _ = subscription_records.remove(&subscription_id);
            }
        } // unlock subscription_records

        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace", skip(subscriber))]
    #[tracing::instrument(name = "subscribe(MemDatabaseset)", level = "info", skip_all)]
    async fn subscribe(
        &self,
        object: Arc<dyn Any + Send + Sync>,
        subscriber: Arc<dyn traits::Subscriber + Send + Sync>,
        subscription_condition: subscription::SubscriptionCondition,
    ) -> result::KuraResult<()> {
        let subscription_id = Uuid::new_v4().to_string();
        let keys = self.get_all_keys();
        let record = SubscriptionRecord::new(
            subscription_id.clone(),
            object,
            subscriber,
            subscription_condition,
            &keys,
        )?;
        let mut subscription_records = self.subscription_records.lock().await;
        record.notify_subscribe_result().await?;
        let _ = subscription_records.insert(subscription_id, record);

        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "unsubscribe(MemDatabaseset)", level = "info", skip_all)]
    async fn unsubscribe(&self, subscribe_id: String) -> result::KuraResult<()> {
        let mut subscription_records = self.subscription_records.lock().await;
        // drop sender object, and let client know the channel is closed
        let _ = subscription_records.remove(&subscribe_id);
        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "delete(MemDatabaseset)", level = "info", skip_all)]
    async fn delete(&self, path: Vec<String>) -> result::KuraResult<()> {
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
                return result::KuraResult::Err(result::KuraError {
                    number: result::KuraErrorCode::NotFound,
                    reason: result::KuraErrorReason::InvalidData,
                    description: Some(format!("{cur_path} not found")),
                });
            }

            target_paths_map.insert(cur_path, target_paths);
        }

        for entry in target_paths_map.iter() {
            let (_, values) = entry.pair();
            for value in values {
                self.metadata_database.remove(value);
                self.actual_state_database.remove(value);
                self.requested_state_database.remove(value);
            }
        }
        Ok(())
    }

    async fn tick(&self, uptime: u128) -> result::KuraResult<()> {
        let mut subscription_records = self.subscription_records.lock().await;
        let mut invalid_subscription_ids = [].to_vec();
        for (subscription_id, record) in subscription_records.iter_mut() {
            let signals: result::KuraResult<Vec<model::SignalStateView>> = record
                .paths
                .lock()
                .await
                .iter()
                .map(|x| self.get_actual_state_view(x.to_string()))
                .collect();
            let signals = signals?;
            let tick_result = record.notify_tick_event(uptime, signals).await;
            if let Err(e) = tick_result {
                error!("tick failed: {}", e);
                invalid_subscription_ids.push(subscription_id.clone());
            }
        }

        for subscription_id in invalid_subscription_ids {
            // drop subscription instance here and close sender.
            let _ = subscription_records.remove(&subscription_id);
        }
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::interface::traits::Database;
    use tokio::sync::Notify;

    const ACTUATOR_PATH: &str = "Vehicle.Test.ActuatorUint8Value";

    fn actuator_metadata() -> model::SignalMetadata {
        model::SignalMetadata {
            leaf_type: model::LeafType::Actuator,
            data_type: model::ValueType::TypeUint8,
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
        }
    }

    fn state(value: u8, is_available_actuator: Option<bool>) -> model::SignalState {
        model::SignalState {
            stored_timestamp: None,
            produced_timestamp: None,
            value: model::Value::Uint8(value),
            is_available_sensor: None,
            is_available_actuator,
        }
    }

    async fn database_reporting_availability(is_available_actuator: Option<bool>) -> MemDatabase {
        let database = MemDatabase::create()
            .unwrap_or_else(|error| panic!("database creation should succeed: {error}"));
        database
            .initialize_signal(ACTUATOR_PATH.to_string(), actuator_metadata())
            .await
            .unwrap_or_else(|error| panic!("signal initialization should succeed: {error}"));
        database
            .set_actual_state(ACTUATOR_PATH.to_string(), state(1, is_available_actuator))
            .await
            .unwrap_or_else(|error| panic!("actual state update should succeed: {error}"));
        database
    }

    enum NotificationBehavior {
        Succeed,
        Fail,
        BlockThenFail {
            entered: Arc<Notify>,
            release: Arc<Notify>,
        },
    }

    struct TestSubscriber {
        behavior: NotificationBehavior,
    }

    impl std::fmt::Debug for TestSubscriber {
        fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
            f.debug_struct("TestSubscriber").finish()
        }
    }

    #[async_trait]
    impl traits::Subscriber for TestSubscriber {
        async fn notify_subscribe_result(
            &self,
            _object: Arc<dyn Any + Send + Sync>,
            _subscription_id: String,
            _ret: bool,
        ) -> result::KuraResult<()> {
            Ok(())
        }

        async fn notify_signals(
            &self,
            _object: Arc<dyn Any + Send + Sync>,
            _subscription_id: String,
            _signals: &[model::SignalStateView],
        ) -> result::KuraResult<()> {
            match &self.behavior {
                NotificationBehavior::Succeed => Ok(()),
                NotificationBehavior::Fail => Err(notification_error()),
                NotificationBehavior::BlockThenFail { entered, release } => {
                    entered.notify_one();
                    release.notified().await;
                    Err(notification_error())
                }
            }
        }
    }

    fn notification_error() -> result::KuraError {
        result::KuraError {
            number: result::KuraErrorCode::BadGateway,
            reason: result::KuraErrorReason::BadGateway,
            description: Some("notification failed".to_string()),
        }
    }

    fn subscription_condition() -> subscription::SubscriptionCondition {
        subscription::SubscriptionCondition {
            mode: subscription::Mode::Actual,
            path: ACTUATOR_PATH.to_string(),
            paths_filter: None,
            misc_filter: Some(subscription::MiscFilter::Timebased(
                subscription::TimebasedFilter { interval: 0 },
            )),
        }
    }

    fn subscription_entry(
        subscription_id: &str,
        behavior: NotificationBehavior,
    ) -> (String, SubscriptionRecord) {
        let subscription_id = subscription_id.to_string();
        let object: Arc<dyn Any + Send + Sync> = Arc::new(());
        let subscriber: Arc<dyn traits::Subscriber + Send + Sync> =
            Arc::new(TestSubscriber { behavior });
        let record = SubscriptionRecord::new(
            subscription_id.clone(),
            object,
            subscriber,
            subscription_condition(),
            [ACTUATOR_PATH.to_string()].as_ref(),
        )
        .unwrap_or_else(|error| panic!("subscription record creation should succeed: {error}"));

        (subscription_id, record)
    }

    #[tokio::test]
    async fn rejects_requested_state_when_actuator_is_unavailable() {
        let database = database_reporting_availability(Some(false)).await;

        let result = database
            .set_requested_state(ACTUATOR_PATH.to_string(), state(2, None))
            .await;

        let error = match result {
            Ok(()) => panic!("requested state update should be rejected"),
            Err(error) => error,
        };
        assert_eq!(error.number, result::KuraErrorCode::ServiceUnavailable);
        assert_eq!(error.reason, result::KuraErrorReason::UnavailableData);
    }

    #[tokio::test]
    async fn accepts_requested_state_when_actuator_availability_is_unreported() {
        let database = database_reporting_availability(None).await;

        let result = database
            .set_requested_state(ACTUATOR_PATH.to_string(), state(2, None))
            .await;

        assert!(result.is_ok());
    }

    #[tokio::test]
    async fn accepts_requested_state_when_actuator_is_available() {
        let database = database_reporting_availability(Some(true)).await;

        let result = database
            .set_requested_state(ACTUATOR_PATH.to_string(), state(2, None))
            .await;

        assert!(result.is_ok());
    }

    #[tokio::test]
    async fn removes_matching_subscription_when_unsubscribing() {
        let database = MemDatabase::create()
            .unwrap_or_else(|error| panic!("database creation should succeed: {error}"));

        {
            let mut records = database.subscription_records.lock().await;
            records.extend([
                subscription_entry("subscription-1", NotificationBehavior::Succeed),
                subscription_entry("target", NotificationBehavior::Succeed),
                subscription_entry("subscription-2", NotificationBehavior::Succeed),
            ]);
        }

        let result = database.unsubscribe("target".to_string()).await;

        assert!(result.is_ok());
        let records = database.subscription_records.lock().await;
        assert_eq!(records.len(), 2);
        assert!(records.contains_key("subscription-1"));
        assert!(records.contains_key("subscription-2"));
        assert!(!records.contains_key("target"));
    }

    #[tokio::test]
    async fn keeps_valid_subscription_when_unsubscribe_waits_during_actual_state_notification() {
        let database = Arc::new(
            MemDatabase::create()
                .unwrap_or_else(|error| panic!("database creation should succeed: {error}")),
        );
        database
            .initialize_signal(ACTUATOR_PATH.to_string(), actuator_metadata())
            .await
            .unwrap_or_else(|error| panic!("signal initialization should succeed: {error}"));

        let entered = Arc::new(Notify::new());
        let release = Arc::new(Notify::new());

        {
            let mut records = database.subscription_records.lock().await;
            records.extend([
                subscription_entry(
                    "failed",
                    NotificationBehavior::BlockThenFail {
                        entered: entered.clone(),
                        release: release.clone(),
                    },
                ),
                subscription_entry("valid", NotificationBehavior::Succeed),
            ]);
        }

        let setter_database = database.clone();
        let set_task = tokio::spawn(async move {
            setter_database
                .set_actual_state(ACTUATOR_PATH.to_string(), state(1, None))
                .await
        });

        // Wait until set_actual_state locks subscription_records.
        entered.notified().await;

        let unsubscribe_database = database.clone();
        let unsubscribe_task =
            tokio::spawn(
                async move { unsubscribe_database.unsubscribe("failed".to_string()).await },
            );

        // Let unsubscribe wait for subscription_records.
        tokio::task::yield_now().await;
        release.notify_one();

        let set_result = set_task
            .await
            .unwrap_or_else(|error| panic!("actual state update task should complete: {error}"));
        let unsubscribe_result = unsubscribe_task
            .await
            .unwrap_or_else(|error| panic!("unsubscribe task should complete: {error}"));

        assert!(set_result.is_ok());
        assert!(unsubscribe_result.is_ok());
        let records = database.subscription_records.lock().await;
        assert_eq!(records.len(), 1);
        assert!(records.contains_key("valid"));
    }

    #[tokio::test]
    async fn removes_only_failed_subscriptions_when_ticking() {
        let database = MemDatabase::create()
            .unwrap_or_else(|error| panic!("database creation should succeed: {error}"));
        database
            .initialize_signal(ACTUATOR_PATH.to_string(), actuator_metadata())
            .await
            .unwrap_or_else(|error| panic!("signal initialization should succeed: {error}"));

        {
            let mut records = database.subscription_records.lock().await;
            records.extend([
                subscription_entry("first-failed", NotificationBehavior::Fail),
                subscription_entry("second-failed", NotificationBehavior::Fail),
                subscription_entry("valid", NotificationBehavior::Succeed),
            ]);
        }

        let result = database.tick(1).await;

        assert!(result.is_ok());
        let records = database.subscription_records.lock().await;
        assert_eq!(records.len(), 1);
        assert!(records.contains_key("valid"));
    }
}
