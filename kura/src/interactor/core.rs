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

use crate::common::{MetadataNode, resolve_relative_path};
use crate::interactor::converter::convert_metadata_to_signal;
use crate::interface::signal::{
    Config, DataPoint, KuraErrorCode, KuraErrorReason, KuraResult, MapExternalToKuraResult, Signal,
};
use crate::interface::subscription::{GetCondition, SubscriptionCondition};
use crate::interface::{Database, Interactor, Subscriber};

use std::any::Any;
use std::collections::HashMap;
use std::fs;
use std::sync::Arc;
use tonic::async_trait;

pub struct InteractorImpl {
    database: Arc<dyn Database + Send + Sync>,
}

impl InteractorImpl {
    pub fn new(database: Arc<dyn Database + Send + Sync>) -> KuraResult<Self> {
        Ok(Self { database })
    }

    fn _get_signals_as_metadatas(&self, vss_path: &String) -> KuraResult<Vec<Signal>> {
        let abs_vss_path = match fs::exists(vss_path) {
            Ok(true) => vss_path.to_string(),
            _ => resolve_relative_path(vss_path)?,
        };
        let json_str = fs::read_to_string(abs_vss_path).to_kura_result(
            KuraErrorCode::NotFound,
            KuraErrorReason::InvalidData,
            format!("{vss_path} not found"),
        )?;
        let root: HashMap<String, MetadataNode> = serde_json::from_str(&json_str).to_kura_result(
            KuraErrorCode::NotFound,
            KuraErrorReason::InvalidData,
            format!("failed to deserialize {vss_path}"),
        )?;
        let mut ret = [].to_vec();
        for (path, node) in root {
            ret.extend(Self::_get_signals(path, &node)?);
        }
        Ok(ret)
    }

    fn _get_signals(path: String, node: &MetadataNode) -> KuraResult<Vec<Signal>> {
        if let MetadataNode::Branch {
            description: _,
            children,
        } = &node
        {
            let mut ret = [].to_vec();
            for (child_path, child_node) in children {
                ret.extend(Self::_get_signals(
                    path.clone() + "." + child_path,
                    child_node,
                )?);
            }
            return Ok(ret);
        }

        Ok([convert_metadata_to_signal(&path, node)?].to_vec())
    }
}

#[async_trait]
impl Interactor for InteractorImpl {
    async fn load_vss(&self, vss_json_path: &str) -> KuraResult<()> {
        let metadata_signals = self._get_signals_as_metadatas(&vss_json_path.to_string())?;
        for metadata_signal in metadata_signals {
            self.database.initialize_signal(&metadata_signal).await?;
        }
        Ok(())
    }

    async fn update_metadata(&self, path: &str, config: &Config) -> KuraResult<()> {
        self.database
            .update_metadata(path.to_string(), config)
            .await?;
        Ok(())
    }

    fn get_current_vehicle_data(&self, get_condition: GetCondition) -> KuraResult<Vec<Signal>> {
        self.database.get_current(get_condition)
    }

    fn get_target_vehicle_data(&self, get_condition: GetCondition) -> KuraResult<Vec<Signal>> {
        self.database.get_target(get_condition)
    }

    async fn set_current_vehicle_data(&self, path: String, datapoint: DataPoint) -> KuraResult<()> {
        self.database.set_current(path, datapoint).await
    }

    async fn set_target_vehicle_data(&self, path: String, datapoint: DataPoint) -> KuraResult<()> {
        self.database.set_target(path, datapoint).await
    }

    async fn subscribe(
        &self,
        object: Arc<dyn Any + Send + Sync>,
        subscriber: Arc<dyn Subscriber + Send + Sync>,
        subscription_condifion: SubscriptionCondition,
    ) -> KuraResult<()> {
        self.database
            .subscribe(object, subscriber, subscription_condifion)
            .await
    }

    async fn unsubscribe(&self, subscribe_id: String) -> KuraResult<()> {
        self.database.unsubscribe(subscribe_id).await
    }

    async fn delete(&self, path: Vec<String>) -> KuraResult<()> {
        self.database.delete(path).await
    }

    async fn tick(&self, uptime: u128) -> KuraResult<()> {
        self.database.tick(uptime).await
    }
}
