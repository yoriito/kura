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

use crate::interface::{result, traits};
use tokio::time::{self, Duration};
use tokio_util::sync::CancellationToken;

use libc::{CLOCK_BOOTTIME, clock_gettime, timespec};

use std::sync::Arc;

use tracing::error;

#[derive(Clone)]
pub struct Tick {
    interactor: Arc<dyn traits::Interactor + Send + Sync>,
    tick_resolution: u64,
}

impl Tick {
    pub fn new(
        interactor: Arc<dyn traits::Interactor + Send + Sync>,
        tick_resolution: u64,
    ) -> result::KuraResult<Self> {
        Ok(Self {
            interactor,
            tick_resolution,
        })
    }

    pub async fn start(&self, cancel: CancellationToken) -> result::KuraResult<()> {
        tokio::select! {
            res = self.tick(&cancel) => res,
            _ = cancel.cancelled() => Ok(())
        }
    }

    async fn tick(&self, cancel: &CancellationToken) -> result::KuraResult<()> {
        let mut interval = time::interval(Duration::from_millis(self.tick_resolution));

        while !cancel.is_cancelled() {
            interval.tick().await;
            if let Err(e) = self.interactor.tick(self.uptime_ms()).await {
                error!("tick event error: {}", e);
            }
        }

        Ok(())
    }

    fn uptime_ms(&self) -> u128 {
        unsafe {
            let mut ts: timespec = std::mem::zeroed();
            clock_gettime(CLOCK_BOOTTIME, &mut ts);
            let dur = Duration::new(ts.tv_sec as u64, ts.tv_nsec as u32);
            dur.as_millis()
        }
    }
}
