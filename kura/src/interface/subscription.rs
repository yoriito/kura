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

use globset::{Glob, GlobMatcher};

use crate::interface::model;
use crate::interface::result::{self, MapExternalToKuraResult};

#[derive(Clone, Debug)]
pub struct GetCondition {
    pub path: String,
    pub paths_filter: Option<PathsFilter>,
}

#[derive(Clone, Debug)]
pub struct SubscriptionCondition {
    pub mode: Mode,
    pub path: String,
    pub paths_filter: Option<PathsFilter>,
    pub misc_filter: Option<MiscFilter>,
}

#[derive(Clone, Debug, PartialEq)]
pub enum Mode {
    Actual,
    Requested,
}

#[derive(Clone, Copy, Debug)]
enum Scalar {
    Bool(bool),
    Int(i128),
    Float(f64),
}

impl SubscriptionCondition {
    pub fn is_match_path(&self, query_path: &String) -> result::KuraResult<bool> {
        let glob = Glob::new(self.path.as_str()).to_kura_result(
            result::KuraErrorCode::BadRequest,
            result::KuraErrorReason::BadRequest,
            format!("invalid glob pattern: {}", self.path),
        )?;
        let matcher: GlobMatcher = glob.compile_matcher();
        if matcher.is_match(query_path) {
            return Ok(true);
        }

        let paths_filter = match &self.paths_filter {
            Some(paths_filter) => paths_filter,
            None => return Ok(false),
        };

        for path_filter in &paths_filter.paths {
            let regex = format!("{}.{}", self.path, path_filter);
            let glob = Glob::new(regex.as_str()).to_kura_result(
                result::KuraErrorCode::BadRequest,
                result::KuraErrorReason::BadRequest,
                format!("invalid glob pattern: {regex}"),
            )?;
            let matcher: GlobMatcher = glob.compile_matcher();
            if matcher.is_match(query_path) {
                return Ok(true);
            }
        }

        Ok(false)
    }
    pub fn get_matched_paths(&self, query_paths: &[String]) -> result::KuraResult<Vec<String>> {
        let mut path_array: Vec<String> = vec![];
        let matcher_array: Vec<GlobMatcher> = self.get_matcher_array()?;

        for path in query_paths {
            for matcher in &matcher_array {
                if matcher.is_match(path) {
                    path_array.push(path.clone());
                    break;
                }
            }
        }
        Ok(path_array)
    }
    pub fn get_matcher_array(&self) -> result::KuraResult<Vec<GlobMatcher>> {
        let mut matcher_array: Vec<GlobMatcher> = vec![];
        let glob = Glob::new(self.path.as_str()).to_kura_result(
            result::KuraErrorCode::BadRequest,
            result::KuraErrorReason::BadRequest,
            format!("invalid glob pattern: {}", self.path),
        )?;
        matcher_array.push(glob.compile_matcher());

        let paths_filter = match &self.paths_filter {
            Some(paths_filter) => paths_filter,
            None => return Ok(matcher_array),
        };

        // TODO: create a matcher in advance
        for path_filter in &paths_filter.paths {
            let regex = format!("{}.{}", self.path, path_filter);
            let glob = Glob::new(regex.as_str()).to_kura_result(
                result::KuraErrorCode::BadRequest,
                result::KuraErrorReason::BadRequest,
                format!("invalid glob pattern: {regex}"),
            )?;
            matcher_array.push(glob.compile_matcher());
        }

        Ok(matcher_array)
    }

    pub fn check_tick_time_filter(&self, previous_time: u128, current_time: u128) -> bool {
        let misc_filter = if let Some(misc_filter) = &self.misc_filter {
            misc_filter
        } else {
            return false;
        };

        let time_based_filter = match misc_filter {
            MiscFilter::Timebased(timebased_filter) => timebased_filter,
            _ => return false,
        };

        current_time - previous_time > time_based_filter.interval as u128
    }

    fn to_scalar(value: &model::Value) -> Option<Scalar> {
        match value.clone() {
            model::Value::Bool(v) => Some(Scalar::Bool(v)),
            model::Value::Int8(v) => Some(Scalar::Int(v as i128)),
            model::Value::Int16(v) => Some(Scalar::Int(v as i128)),
            model::Value::Int32(v) => Some(Scalar::Int(v as i128)),
            model::Value::Int64(v) => Some(Scalar::Int(v as i128)),
            model::Value::Uint8(v) => Some(Scalar::Int(v as i128)),
            model::Value::Uint16(v) => Some(Scalar::Int(v as i128)),
            model::Value::Uint32(v) => Some(Scalar::Int(v as i128)),
            model::Value::Uint64(v) => Some(Scalar::Int(v as i128)),
            model::Value::Float(v) => Some(Scalar::Float(v as f64)),
            model::Value::Double(v) => Some(Scalar::Float(v)),
            _ => None,
        }
    }

    fn compare_bool(opcode: &Opcode, pre: bool, cur: bool) -> bool {
        match opcode {
            Opcode::Equal => pre == cur,
            Opcode::NotEqual => pre != cur,
            Opcode::GreaterThan => !pre && cur,
            Opcode::LesserThan => pre && !cur,
            _ => true,
        }
    }

    fn compare_int(opcode: &Opcode, offset: i32, pre: i128, cur: i128) -> bool {
        let adjusted = cur - offset as i128;
        match opcode {
            Opcode::Equal => adjusted == pre,
            Opcode::NotEqual => adjusted != pre,
            Opcode::GreaterThan => adjusted > pre,
            Opcode::GreaterEqual => adjusted >= pre,
            Opcode::LesserThan => adjusted < pre,
            Opcode::LesserEqual => adjusted <= pre,
            _ => true,
        }
    }

    fn compare_float(opcode: &Opcode, offset: i32, pre: f64, cur: f64) -> bool {
        let adjusted = cur - offset as f64;
        match opcode {
            Opcode::Equal => adjusted == pre,
            Opcode::NotEqual => adjusted != pre,
            Opcode::GreaterThan => adjusted > pre,
            Opcode::GreaterEqual => adjusted >= pre,
            Opcode::LesserThan => adjusted < pre,
            Opcode::LesserEqual => adjusted <= pre,
            _ => true,
        }
    }

    fn is_changed(
        &self,
        opcode: &Opcode,
        offset: i32,
        pre_value: &model::Value,
        cur_value: &model::Value,
    ) -> bool {
        match (Self::to_scalar(pre_value), Self::to_scalar(cur_value)) {
            (Some(Scalar::Bool(pre)), Some(Scalar::Bool(cur))) => {
                return Self::compare_bool(opcode, pre, cur);
            }
            (Some(Scalar::Int(pre)), Some(Scalar::Int(cur))) => {
                return Self::compare_int(opcode, offset, pre, cur);
            }
            (Some(Scalar::Float(pre)), Some(Scalar::Float(cur))) => {
                return Self::compare_float(opcode, offset, pre, cur);
            }
            _ => (),
        }
        match (pre_value, cur_value) {
            (model::Value::Complex(a), model::Value::Complex(b)) => match opcode {
                Opcode::Equal => a == b,
                Opcode::NotEqual => a != b,
                _ => true,
            },
            _ => true,
        }
    }

    pub fn check_change_filter(
        &self,
        previous_notified_state: &Option<model::SignalState>,
        current_state: &model::SignalState,
    ) -> bool {
        let misc_filter = if let Some(misc_filter) = &self.misc_filter {
            misc_filter
        } else {
            return true;
        };

        let change_filter = match misc_filter {
            MiscFilter::Change(change_filter) => change_filter,
            _ => return true,
        };

        let previous_notified_state = if let Some(previous_notified_state) = previous_notified_state
        {
            previous_notified_state
        } else {
            return true;
        };

        self.is_changed(
            &change_filter.opcode,
            change_filter.offset,
            &previous_notified_state.value,
            &current_state.value,
        )
    }
}

impl GetCondition {
    pub fn get_matched_paths(&self, query_paths: &[String]) -> result::KuraResult<Vec<String>> {
        let mut path_array: Vec<String> = vec![];
        let matcher_array: Vec<GlobMatcher> = self.get_matcher_array()?;

        for path in query_paths {
            for matcher in &matcher_array {
                if matcher.is_match(path) {
                    path_array.push(path.clone());
                    break;
                }
            }
        }
        Ok(path_array)
    }
    pub fn get_matcher_array(&self) -> result::KuraResult<Vec<GlobMatcher>> {
        let mut matcher_array: Vec<GlobMatcher> = vec![];
        let glob = Glob::new(self.path.as_str()).to_kura_result(
            result::KuraErrorCode::BadRequest,
            result::KuraErrorReason::BadRequest,
            format!("invalid glob pattern: {}", self.path),
        )?;
        matcher_array.push(glob.compile_matcher());

        let paths_filter = match &self.paths_filter {
            Some(paths_filter) => paths_filter,
            None => return Ok(matcher_array),
        };

        for path_filter in &paths_filter.paths {
            let regex = format!("{}.{}", self.path, path_filter);
            let glob = Glob::new(regex.as_str()).to_kura_result(
                result::KuraErrorCode::BadRequest,
                result::KuraErrorReason::BadRequest,
                format!("invalid glob pattern: {regex}"),
            )?;
            matcher_array.push(glob.compile_matcher());
        }

        Ok(matcher_array)
    }
}

#[derive(Clone, Debug)]
pub enum MiscFilter {
    Timebased(TimebasedFilter),
    #[allow(dead_code)]
    Range(RangeFilter),
    Change(ChangeFilter),
    #[allow(dead_code)]
    Curvelog(CurvelogFilter),
    #[allow(dead_code)]
    History(HistoryFilter),
    #[allow(dead_code)]
    Metadata(MetadataFilter),
    #[allow(dead_code)]
    Invalid,
}

#[derive(Clone, Debug)]
pub struct PathsFilter {
    pub paths: Vec<String>,
}

#[derive(Clone, Debug)]
pub struct TimebasedFilter {
    pub interval: u32,
}

#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct RangeFilter {
    pub conditions: Vec<RangeCondition>,
    pub combination: Option<Combination>,
}

#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct RangeCondition {
    pub opcode: Opcode,
    pub boundary: i32,
}

#[derive(Clone, Debug)]
pub struct ChangeFilter {
    pub opcode: Opcode,
    pub offset: i32,
}

#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct CurvelogFilter {
    pub maxerr: f32,
    pub bufsize: u32,
}

#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct HistoryFilter {
    pub time_period: String,
}

#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct MetadataFilter {
    pub metadata_key: String,
}

#[derive(Clone, Debug)]
pub enum Opcode {
    Undefined = 0,
    Equal = 1,
    NotEqual = 2,
    GreaterThan = 3,
    GreaterEqual = 4,
    LesserThan = 5,
    LesserEqual = 6,
}

#[derive(Clone, Debug)]
pub enum Combination {
    Undefined = 0,
    And = 1,
    Or = 2,
}
