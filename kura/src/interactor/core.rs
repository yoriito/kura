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

use crate::common::resolve_relative_path;
use crate::interactor::struct_filter;
use crate::interactor::vss_loader;
use crate::interface::{
    model,
    result::{self, MapExternalToKuraResult},
    subscription, traits,
};

use std::any::Any;
use std::collections::HashMap;
use std::fs;
use std::sync::Arc;
use tonic::async_trait;

pub struct InteractorImpl {
    database: Arc<dyn traits::Database + Send + Sync>,
}

impl InteractorImpl {
    pub fn new(database: Arc<dyn traits::Database + Send + Sync>) -> result::KuraResult<Self> {
        Ok(Self { database })
    }
}

#[async_trait]
impl traits::Interactor for InteractorImpl {
    async fn load_vss(&self, vss_json_path: &str) -> result::KuraResult<()> {
        let abs_vss_path = match fs::exists(vss_json_path) {
            Ok(true) => vss_json_path.to_string(),
            _ => resolve_relative_path(vss_json_path)?,
        };
        let json_str = fs::read_to_string(abs_vss_path).to_kura_result(
            result::KuraErrorCode::NotFound,
            result::KuraErrorReason::InvalidData,
            format!("{vss_json_path} not found"),
        )?;

        let vss_user_defined_types =
            vss_loader::load_user_defined_types(&vss_json_path.to_string(), &json_str)?;
        let signal_metadatas = vss_loader::load_metadata(
            &vss_json_path.to_string(),
            &json_str,
            &vss_user_defined_types,
        )?;
        self.database
            .initialize_vss_user_defined_types(vss_user_defined_types)
            .await?;
        for (path, signal_metadata) in signal_metadatas {
            self.database
                .initialize_signal(path, signal_metadata)
                .await?;
        }
        Ok(())
    }

    async fn update_metadata(
        &self,
        path: &str,
        metadata: &model::SignalMetadata,
    ) -> result::KuraResult<()> {
        self.database
            .update_metadata(path.to_string(), metadata)
            .await?;
        Ok(())
    }

    fn get_actual_state(
        &self,
        get_condition: subscription::GetCondition,
    ) -> result::KuraResult<Vec<model::SignalStateView>> {
        self.database.get_actual_state(get_condition)
    }

    fn get_requested_state(
        &self,
        get_condition: subscription::GetCondition,
    ) -> result::KuraResult<Vec<model::SignalStateView>> {
        self.database.get_requested_state(get_condition)
    }

    fn get_metadata(
        &self,
        get_condition: subscription::GetCondition,
    ) -> result::KuraResult<Vec<model::SignalMetadataView>> {
        self.database.get_metadata(get_condition)
    }

    fn get_vss_user_defined_types_referenced_by(
        &self,
        signals: &[model::SignalMetadataView],
    ) -> result::KuraResult<HashMap<String, model::VSSUserDefinedType>> {
        let vss_user_defined_types = self.database.get_all_vss_user_defined_types()?;
        Ok(struct_filter::filter_referenced_struct_types(
            signals,
            &vss_user_defined_types,
        ))
    }

    async fn initialize_vss_user_defined_types(
        &self,
        vss_user_defined_types: Vec<model::VSSUserDefinedType>,
    ) -> result::KuraResult<bool> {
        self.database
            .initialize_vss_user_defined_types(vss_user_defined_types)
            .await
    }

    async fn set_actual_state(
        &self,
        path: String,
        state: model::SignalState,
    ) -> result::KuraResult<()> {
        self.database.set_actual_state(path, state).await
    }

    async fn set_requested_state(
        &self,
        path: String,
        state: model::SignalState,
    ) -> result::KuraResult<()> {
        self.database.set_requested_state(path, state).await
    }

    async fn subscribe(
        &self,
        object: Arc<dyn Any + Send + Sync>,
        subscriber: Arc<dyn traits::Subscriber + Send + Sync>,
        subscription_condifion: subscription::SubscriptionCondition,
    ) -> result::KuraResult<()> {
        self.database
            .subscribe(object, subscriber, subscription_condifion)
            .await
    }

    async fn unsubscribe(&self, subscribe_id: String) -> result::KuraResult<()> {
        self.database.unsubscribe(subscribe_id).await
    }

    async fn delete(&self, path: Vec<String>) -> result::KuraResult<()> {
        self.database.delete(path).await
    }

    async fn tick(&self, uptime: u128) -> result::KuraResult<()> {
        self.database.tick(uptime).await
    }
}
