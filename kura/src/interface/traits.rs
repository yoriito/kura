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

use async_trait::async_trait;
use std::any::Any;
use std::collections::HashMap;
use std::sync::Arc;

use crate::interface::model;
use crate::interface::result::KuraResult;
use crate::interface::subscription::{GetCondition, SubscriptionCondition};

// Interfactor interfaces declares all functions provided by this software.
// Therefore, name the interface in terms of all the functions in this software.
#[async_trait]
pub trait Interactor: Send + Sync {
    async fn load_vss(&self, vss_json_path: &str) -> KuraResult<()>;
    async fn update_metadata(&self, path: &str, metadata: &model::SignalMetadata)
    -> KuraResult<()>;
    fn get_actual_state(
        &self,
        get_condition: GetCondition,
    ) -> KuraResult<Vec<model::SignalStateView>>;
    fn get_requested_state(
        &self,
        get_condition: GetCondition,
    ) -> KuraResult<Vec<model::SignalStateView>>;
    fn get_metadata(
        &self,
        get_condition: GetCondition,
    ) -> KuraResult<Vec<model::SignalMetadataView>>;
    fn get_vss_user_defined_types_referenced_by(
        &self,
        signals: &[model::SignalMetadataView],
    ) -> KuraResult<HashMap<String, model::VSSUserDefinedType>>;
    async fn initialize_vss_user_defined_types(
        &self,
        vss_user_defined_types: Vec<model::VSSUserDefinedType>,
    ) -> KuraResult<bool>;
    async fn set_actual_state(&self, path: String, state: model::SignalState) -> KuraResult<()>;
    async fn set_requested_state(&self, path: String, state: model::SignalState) -> KuraResult<()>;
    async fn subscribe(
        &self,
        object: Arc<dyn Any + Send + Sync>,
        subscriber: Arc<dyn Subscriber + Send + Sync>,
        subscription_condition: SubscriptionCondition,
    ) -> KuraResult<()>;
    async fn unsubscribe(&self, subscribe_id: String) -> KuraResult<()>;
    async fn delete(&self, path: Vec<String>) -> KuraResult<()>;
    async fn tick(&self, uptime: u128) -> KuraResult<()>;
}

#[async_trait]
pub trait Database: Send + Sync {
    async fn initialize_signal(
        &self,
        path: String,
        signal: model::SignalMetadata,
    ) -> KuraResult<bool>;
    async fn initialize_vss_user_defined_types(
        &self,
        vss_user_defined_types: Vec<model::VSSUserDefinedType>,
    ) -> KuraResult<bool>;
    async fn update_metadata(
        &self,
        path: String,
        metadata: &model::SignalMetadata,
    ) -> KuraResult<bool>;
    fn get_actual_state(
        &self,
        get_condition: GetCondition,
    ) -> KuraResult<Vec<model::SignalStateView>>;

    #[allow(dead_code)]
    fn get_requested_state(
        &self,
        get_condition: GetCondition,
    ) -> KuraResult<Vec<model::SignalStateView>>;
    fn get_metadata(
        &self,
        get_condition: GetCondition,
    ) -> KuraResult<Vec<model::SignalMetadataView>>;
    fn get_all_vss_user_defined_types(
        &self,
    ) -> KuraResult<HashMap<String, model::VSSUserDefinedType>>;
    async fn set_actual_state(&self, path: String, state: model::SignalState) -> KuraResult<()>;
    async fn set_requested_state(&self, path: String, state: model::SignalState) -> KuraResult<()>;
    async fn subscribe(
        &self,
        object: Arc<dyn Any + Send + Sync>,
        subscriber: Arc<dyn Subscriber + Send + Sync>,
        subscription_condition: SubscriptionCondition,
    ) -> KuraResult<()>;
    async fn unsubscribe(&self, subscribe_id: String) -> KuraResult<()>;
    async fn delete(&self, path: Vec<String>) -> KuraResult<()>;
    async fn tick(&self, uptime: u128) -> KuraResult<()>;
}

#[async_trait]
pub trait Subscriber: Send + Sync + std::fmt::Debug {
    async fn notify_subscribe_result(
        &self,
        object: Arc<dyn Any + Send + Sync>,
        subscription_id: String,
        ret: bool,
    ) -> KuraResult<()>;
    async fn notify_signals(
        &self,
        object: Arc<dyn Any + Send + Sync>,
        subscription_id: String,
        signals: &[model::SignalStateView],
    ) -> KuraResult<()>;
}
