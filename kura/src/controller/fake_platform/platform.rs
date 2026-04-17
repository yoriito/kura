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

use crate::interface::Interactor;
use crate::interface::Subscriber;
use crate::interface::signal::KuraResult;
use crate::interface::subscription::{self};
use std::time::{SystemTime, UNIX_EPOCH};

use tokio::sync::RwLock;
use tokio_util::sync::CancellationToken;

use async_trait::async_trait;
use std::any::Any;
use std::sync::Arc;

use tokio::time::{self, Duration};

pub struct FakePlatform {
    interactor: Arc<dyn Interactor + Send + Sync>,
    request_signals: Arc<RwLock<Vec<crate::interface::signal::Signal>>>,
}

impl FakePlatform {
    pub fn new(interactor: Arc<dyn Interactor + Send + Sync>) -> KuraResult<Self> {
        Ok(Self {
            interactor,
            request_signals: Arc::new(RwLock::new(Vec::new())),
        })
    }
    pub async fn start(&self, cancel: CancellationToken) -> KuraResult<()> {
        tokio::select! {
            res = self.tick(&cancel) => res?,
            _ = cancel.cancelled() => {}
        }
        Ok(())
    }

    async fn tick(&self, cancel: &CancellationToken) -> KuraResult<()> {
        self.interactor
            .subscribe(
                Arc::new(()),
                Arc::new(SubscribrFakePlatform {
                    request_signals: self.request_signals.clone(),
                }),
                subscription::SubscriptionCondition {
                    mode: subscription::Mode::Target,
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
                    .set_current_vehicle_data(signal.path.clone(), signal.target_datapoint.clone())
                    .await?;
            }
            self.request_signals.write().await.clear();
        }
        Ok(())
    }
}

struct SubscribrFakePlatform {
    request_signals: Arc<RwLock<Vec<crate::interface::signal::Signal>>>,
}

impl std::fmt::Debug for SubscribrFakePlatform {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("SubscribrFakePlatform").finish()
    }
}

#[async_trait]
impl Subscriber for SubscribrFakePlatform {
    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(
        name = "notify_subscribe_result(SubscribrYoriito)",
        level = "info",
        skip_all
    )]
    async fn notify_subscribe_result(
        &self,
        _: Arc<dyn Any + Send + Sync>,
        _: String,
        _: bool,
    ) -> KuraResult<()> {
        Ok(())
    }

    #[tracing::instrument(name = "args", level = "trace")]
    #[tracing::instrument(name = "notify_signals(SubscribrYoriito)", level = "info", skip_all)]
    async fn notify_signals(
        &self,
        _: Arc<dyn Any + Send + Sync>,
        _: String,
        signals: &[crate::interface::signal::Signal],
    ) -> KuraResult<()> {
        for signal in signals {
            self.request_signals.write().await.push(signal.clone());
        }

        Ok(())
    }
}

#[allow(dead_code)]
fn get_time() -> u64 {
    let now = SystemTime::now();
    let duration_since_epoch = now.duration_since(UNIX_EPOCH).unwrap();
    duration_since_epoch.as_secs()
}
