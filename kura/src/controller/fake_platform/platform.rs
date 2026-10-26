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

use crate::interface::{model, result, subscription, traits};

use tokio::sync::RwLock;
use tokio_util::sync::CancellationToken;

use async_trait::async_trait;
use std::any::Any;
use std::sync::Arc;

use tokio::time::{self, Duration};

pub struct FakePlatform {
    interactor: Arc<dyn traits::Interactor + Send + Sync>,
    request_signals: Arc<RwLock<Vec<model::SignalStateView>>>,
}

impl FakePlatform {
    pub fn new(interactor: Arc<dyn traits::Interactor + Send + Sync>) -> result::KuraResult<Self> {
        Ok(Self {
            interactor,
            request_signals: Arc::new(RwLock::new(Vec::new())),
        })
    }
    pub async fn start(&self, cancel: CancellationToken) -> result::KuraResult<()> {
        tokio::select! {
            res = self.tick(&cancel) => res?,
            _ = cancel.cancelled() => {}
        }
        Ok(())
    }

    async fn tick(&self, cancel: &CancellationToken) -> result::KuraResult<()> {
        self.interactor
            .subscribe(
                Arc::new(()),
                Arc::new(SubscriberFakePlatform {
                    request_signals: self.request_signals.clone(),
                }),
                subscription::SubscriptionCondition {
                    mode: subscription::Mode::Requested,
                    path: "*".to_string(),
                    paths_filter: None,
                    misc_filter: None,
                },
            )
            .await?;

        let mut interval = time::interval(Duration::from_millis(1000));
        while !cancel.is_cancelled() {
            interval.tick().await;
            for signal in self.request_signals.read().await.iter() {
                self.interactor
                    .set_actual_state(signal.path.clone(), signal.state.clone())
                    .await?;
            }
            self.request_signals.write().await.clear();
        }
        Ok(())
    }
}

struct SubscriberFakePlatform {
    request_signals: Arc<RwLock<Vec<model::SignalStateView>>>,
}

impl std::fmt::Debug for SubscriberFakePlatform {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("SubscriberFakePlatform").finish()
    }
}

#[async_trait]
impl traits::Subscriber for SubscriberFakePlatform {
    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(
        name = "notify_subscribe_result(SubscriberFakePlatform)",
        level = "info",
        skip_all
    )]
    async fn notify_subscribe_result(
        &self,
        _: Arc<dyn Any + Send + Sync>,
        _: String,
        _: bool,
    ) -> result::KuraResult<()> {
        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(
        name = "notify_signals(SubscriberFakePlatform)",
        level = "info",
        skip_all
    )]
    async fn notify_signals(
        &self,
        _: Arc<dyn Any + Send + Sync>,
        _: String,
        signals: &[model::SignalStateView],
    ) -> result::KuraResult<()> {
        let mut guard = self.request_signals.write().await;
        guard.extend(signals.iter().cloned());

        Ok(())
    }
}
