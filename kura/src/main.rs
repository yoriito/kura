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

mod args;
mod common;
mod controller;
mod entity;
mod interactor;
mod interface;

use std::sync::Arc;

use clap::Parser;
use tokio::task::JoinSet;
use tokio_util::sync::CancellationToken;

use crate::common::Config;

use crate::controller::fake_platform::FakePlatform;
use crate::controller::yoriito::consumer_server::YoriitoConsumerServer;
use crate::controller::yoriito::producer_server::YoriitoProducerServer;

use crate::controller::tick::Tick;

#[cfg(feature = "memdb")]
use crate::entity::mem_database::MemDatabase as Database;
#[cfg(not(feature = "memdb"))]
use crate::entity::sled_database::SledDatabase as Database;

use crate::interactor::InteractorImpl;
use crate::interface::Interactor;
use crate::interface::signal::{KuraError, KuraErrorCode, KuraErrorReason, KuraResult};
use crate::{args::Args, interface::subscription::GetCondition};

use tracing::error;
use tracing_subscriber::{EnvFilter, FmtSubscriber, fmt::format::FmtSpan};

#[allow(dead_code)]
const GRPC_PORT_PATH: &str = "Server.Config.Protocol.Grpc.Protobuf.PortNum";
#[allow(dead_code)]
const WEBSOCKET_PORT_PATH: &str = "Server.Config.Protocol.Websocket.Primary.PortNum";
const YORIITO_CONSUMER_PORT_PATH: &str = "Server.Config.Protocol.Yoriito.ConsumerPortNum";
const YORIITO_PRODUCER_PORT_PATH: &str = "Server.Config.Protocol.Yoriito.ProviderPortNum";

#[tokio::main(flavor = "multi_thread")]
async fn main() -> Result<(), anyhow::Error> {
    let args = Args::parse();
    setup_logging(&args);

    let config = match Config::load(args.config_file_path.as_str()) {
        Ok(config) => config,
        Err(error) => {
            error!("{error}");
            std::process::exit(1);
        }
    };

    let database = match Database::create() {
        Ok(database) => database,
        Err(error) => {
            error!("{error}");
            std::process::exit(1);
        }
    };
    let database = Arc::new(database);
    let interactor = match InteractorImpl::new(database) {
        Ok(interactor) => interactor,
        Err(error) => {
            error!("{error}");
            std::process::exit(1);
        }
    };
    let interactor = Arc::new(interactor);
    if let Err(error) = initialize_db_by_vss_file(
        [config.vss_path, config.server_path].to_vec(),
        interactor.clone(),
    )
    .await
    {
        error!("{error}");
        std::process::exit(1);
    };

    let cancel_token = CancellationToken::new();
    let mut task_set: JoinSet<KuraResult<()>> = JoinSet::new();

    if let Err(error) = spawn_tick(
        config.tick_resolution,
        &mut task_set,
        interactor.clone(),
        &cancel_token,
    )
    .await
    {
        error!("{error}");
        std::process::exit(1);
    }

    #[cfg(feature = "yoriito")]
    if let Err(error) = spawn_yoriito(&mut task_set, interactor.clone(), &cancel_token).await {
        error!("{error}");
        std::process::exit(1);
    }

    #[cfg(feature = "fake_platform")]
    if let Err(error) = spawn_fakeplatform(&args, &mut task_set, interactor, &cancel_token) {
        error!("{error}");
        std::process::exit(1);
    }

    tokio::select! {
        _ = tokio::signal::ctrl_c() => {
            cancel_token.cancel();
        }
        Some(e) = task_set.join_next() => {
            match e {
                Ok(Ok(_)) => (),
                Ok(Err(error)) => error!("{error}"),
                Err(error) => error!("{error}"),
            }
            cancel_token.cancel();
        }
    };

    Ok(())
}

async fn initialize_db_by_vss_file(
    vss_file_paths: Vec<String>,
    interactor: Arc<InteractorImpl>,
) -> KuraResult<()> {
    for vss_file_path in vss_file_paths {
        interactor.load_vss(&vss_file_path).await?;
    }
    Ok(())
}

async fn spawn_tick(
    tick_resolution: u64,
    task_set: &mut JoinSet<KuraResult<()>>,
    interactor: Arc<InteractorImpl>,
    cancel_token: &CancellationToken,
) -> KuraResult<()> {
    let tick_provider = Arc::new(Tick::new(interactor.clone(), tick_resolution)?);
    let cancel_cloned = cancel_token.clone();
    task_set.spawn(async move { tick_provider.start(cancel_cloned).await });
    Ok(())
}

#[cfg(feature = "yoriito")]
async fn spawn_yoriito(
    set: &mut JoinSet<KuraResult<()>>,
    interactor: Arc<InteractorImpl>,
    cancel: &CancellationToken,
) -> KuraResult<()> {
    let condition = GetCondition {
        path: YORIITO_CONSUMER_PORT_PATH.to_string(),
        paths_filter: None,
    };
    let signals = interactor.get_current_vehicle_data(condition)?;
    if signals.len() != 1 {
        return KuraResult::Err(KuraError {
            number: KuraErrorCode::NotFound,
            reason: KuraErrorReason::InvalidData,
            description: Some(format!("{YORIITO_CONSUMER_PORT_PATH} not found")),
        });
    }
    let signal = &signals[0];
    let yoriito_consumer_addr = format!("127.0.0.1:{}", signal.current_datapoint.value);

    let condition = GetCondition {
        path: YORIITO_PRODUCER_PORT_PATH.to_string(),
        paths_filter: None,
    };
    let signals = interactor.get_current_vehicle_data(condition)?;
    if signals.len() != 1 {
        return KuraResult::Err(KuraError {
            number: KuraErrorCode::NotFound,
            reason: KuraErrorReason::InvalidData,
            description: Some(format!("{YORIITO_PRODUCER_PORT_PATH} not found")),
        });
    }
    let signal = &signals[0];
    let yoriito_producer_addr = format!("127.0.0.1:{}", signal.current_datapoint.value);
    let yoriito_consumer_server = Arc::new(YoriitoConsumerServer::new(
        &yoriito_consumer_addr,
        interactor.clone(),
    )?);
    let yoriito_producer_server = Arc::new(YoriitoProducerServer::new(
        &yoriito_producer_addr,
        interactor.clone(),
    )?);
    let cancel_cloned = cancel.clone();
    set.spawn(async move { yoriito_consumer_server.start(cancel_cloned).await });
    let cancel_cloned = cancel.clone();
    set.spawn(async move { yoriito_producer_server.start(cancel_cloned).await });
    Ok(())
}

#[cfg(feature = "fake_platform")]
fn spawn_fakeplatform(
    args: &Args,
    set: &mut JoinSet<KuraResult<()>>,
    interactor: Arc<InteractorImpl>,
    cancel: &CancellationToken,
) -> KuraResult<()> {
    if !args.fake_platform {
        return Ok(());
    }

    let fake_platform = Arc::new(FakePlatform::new(interactor.clone())?);
    let cancel_cloned = cancel.clone();
    set.spawn(async move { fake_platform.start(cancel_cloned).await });

    Ok(())
}

fn setup_logging(args: &Args) {
    let this_crate = env!("CARGO_PKG_NAME").replace('-', "_");
    //let level = if args.verbose { "trace" } else { "info" };
    let level: &str = if args.verbose { "trace" } else { "error" };
    let directive = format!("{this_crate}={level}");
    let subscriber = FmtSubscriber::builder()
        .with_env_filter(EnvFilter::new(directive))
        .with_span_events(FmtSpan::ENTER | FmtSpan::EXIT)
        .finish();

    tracing::subscriber::set_global_default(subscriber).expect("setting default subscriber failed");
}
