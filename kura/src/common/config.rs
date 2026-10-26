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

use crate::interface::result::{self, MapExternalToKuraResult};
use serde::Deserialize;

use std::env;
use std::fs;
use std::fs::File;
use std::io::BufReader;

#[derive(Debug, Deserialize)]
pub struct Config {
    pub vss_path: String,
    pub server_path: String,
    pub tick_resolution: u64,
}

impl Config {
    pub fn load(config_file_path: &str) -> result::KuraResult<Self> {
        let config_file_path = match fs::exists(config_file_path) {
            Ok(true) => config_file_path.to_string(),
            _ => resolve_relative_path(config_file_path)?,
        };
        let config_file = File::open(config_file_path.clone()).to_kura_result(
            result::KuraErrorCode::NotFound,
            result::KuraErrorReason::InvalidData,
            format!("failed to open {config_file_path}"),
        )?;
        let config_reader = BufReader::new(config_file);
        let ret: Config = serde_yaml::from_reader(config_reader).to_kura_result(
            result::KuraErrorCode::NotFound,
            result::KuraErrorReason::InvalidData,
            format!("failed to deserialize {config_file_path}"),
        )?;
        Ok(ret)
    }
}

pub fn resolve_relative_path(relative_path: &str) -> result::KuraResult<String> {
    let home_dir_path = match env::home_dir() {
        Some(home_dir_path) => home_dir_path.display().to_string(),
        None => "/home/root".to_string(),
    };
    let current_dir_path = match env::current_dir() {
        Ok(current_dir_path) => current_dir_path.display().to_string(),
        Err(_) => "".to_string(),
    };
    let candidate_parents = [
        home_dir_path,
        "/etc/yoriito/viss".to_string(),
        "/usr/local/etc/yoriito/viss".to_string(),
        current_dir_path,
    ]
    .to_vec();

    let mut candidate_paths = candidate_parents
        .iter()
        .map(|candidate_parent| candidate_parent.clone() + "/" + relative_path);
    match candidate_paths.find(|candidate_path| fs::exists(candidate_path).unwrap_or_default()) {
        Some(path) => result::KuraResult::Ok(path),
        None => result::KuraResult::Err(result::KuraError {
            number: result::KuraErrorCode::NotFound,
            reason: result::KuraErrorReason::InvalidData,
            description: Some(format!("{relative_path} not found")),
        }),
    }
}
