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

use crate::interface::signal::{KuraError, MapExternalToKuraResult};
use crate::interface::signal::{KuraErrorCode, KuraErrorReason, KuraResult};
use serde::Deserialize;
use serde::Serialize;

use std::env;
use std::fs;
use std::fs::File;
use std::io::BufReader;

use std::collections::HashMap;

#[derive(Debug, Deserialize)]
pub struct Config {
    pub vss_path: String,
    pub server_path: String,
    pub tick_resolution: u64,
}

impl Config {
    pub fn load(config_file_path: &str) -> KuraResult<Self> {
        let config_file_path = match fs::exists(config_file_path) {
            Ok(true) => config_file_path.to_string(),
            _ => resolve_relative_path(config_file_path)?,
        };
        let config_file = File::open(config_file_path.clone()).to_kura_result(
            KuraErrorCode::NotFound,
            KuraErrorReason::InvalidData,
            format!("failed to open {config_file_path}"),
        )?;
        let config_reader = BufReader::new(config_file);
        let ret: Config = serde_yaml::from_reader(config_reader).to_kura_result(
            KuraErrorCode::NotFound,
            KuraErrorReason::InvalidData,
            format!("failed to deserialize {config_file_path}"),
        )?;
        Ok(ret)
    }
}

#[derive(Clone, serde::Serialize, Debug, Deserialize)]
#[serde(untagged)]
pub enum FlexibleValue {
    Int(i64),
    Float(f64),
    Str(String),
    Bool(bool),
    Null,
    Array(Vec<FlexibleValue>),
    Object(std::collections::HashMap<String, FlexibleValue>),
}

#[derive(Debug, Serialize, Deserialize)]
#[serde(tag = "type")]
pub enum MetadataNode {
    #[serde(rename = "branch")]
    Branch {
        description: Option<String>,
        #[serde(default)]
        children: HashMap<String, MetadataNode>,
    },

    #[serde(rename = "attribute")]
    AttributeMetadata {
        datatype: String,
        description: Option<String>,
        #[serde(default)]
        deprecation: Option<String>,
        #[serde(default)]
        comment: Option<String>,
        #[serde(default)]
        min: Option<FlexibleValue>,
        #[serde(default)]
        max: Option<FlexibleValue>,
        #[serde(default)]
        unit: Option<String>,
        #[serde(default)]
        pattern: Option<String>,
        #[serde(default)]
        default: Option<FlexibleValue>,
        #[serde(default)]
        allowed: Vec<FlexibleValue>,
        //allowed: Vec<String>,
        #[serde(default)]
        arraysize: u64,
        #[serde(default)]
        user_defined_items: HashMap<String, String>,
    },

    #[serde(rename = "sensor")]
    SensorMetadata {
        datatype: String,
        description: Option<String>,
        #[serde(default)]
        deprecation: Option<String>,
        #[serde(default)]
        comment: Option<String>,
        #[serde(default)]
        min: Option<FlexibleValue>,
        #[serde(default)]
        max: Option<FlexibleValue>,
        #[serde(default)]
        unit: Option<String>,
        #[serde(default)]
        pattern: Option<String>,
        #[serde(default)]
        default: Option<FlexibleValue>,
        #[serde(default)]
        allowed: Vec<FlexibleValue>,
        //allowed: Vec<String>,
        #[serde(default)]
        arraysize: u64,
    },

    #[serde(rename = "actuator")]
    ActuatorMetadata {
        datatype: String,
        description: Option<String>,
        #[serde(default)]
        deprecation: Option<String>,
        #[serde(default)]
        comment: Option<String>,
        #[serde(default)]
        min: Option<FlexibleValue>,
        #[serde(default)]
        max: Option<FlexibleValue>,
        #[serde(default)]
        unit: Option<String>,
        #[serde(default)]
        pattern: Option<String>,
        #[serde(default)]
        default: Option<FlexibleValue>,
        #[serde(default)]
        allowed: Vec<FlexibleValue>,
        //allowed: Vec<String>,
        #[serde(default)]
        arraysize: u64,
    },
}

pub fn resolve_relative_path(relative_path: &str) -> KuraResult<String> {
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
        Some(path) => KuraResult::Ok(path),
        None => KuraResult::Err(KuraError {
            number: KuraErrorCode::NotFound,
            reason: KuraErrorReason::InvalidData,
            description: Some(format!("{relative_path} not found")),
        }),
    }
}
