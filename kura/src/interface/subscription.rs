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

use crate::interface::signal::Value;
use tracing::error;

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
    Current,
    Target,
}

impl SubscriptionCondition {
    pub fn is_match_path(&self, query_path: &String) -> bool {
        let glob = if let Ok(glob) = Glob::new(self.path.as_str()) {
            glob
        } else {
            return false;
        };
        let matcher: GlobMatcher = glob.compile_matcher();
        if matcher.is_match(query_path) {
            return true;
        }

        let paths_filter = match &self.paths_filter {
            Some(paths_filter) => paths_filter,
            None => return false,
        };

        for path_filter in &paths_filter.paths {
            let regex = format!("{}.{}", self.path, path_filter);
            let glob = Glob::new(regex.as_str()).unwrap();
            let matcher: GlobMatcher = glob.compile_matcher();
            if matcher.is_match(query_path) {
                return true;
            }
        }

        false
    }
    pub fn get_matched_paths(&self, query_paths: &[String]) -> Vec<String> {
        let mut path_array: Vec<String> = vec![];
        let matcher_array: Vec<GlobMatcher> = self.get_matcher_array();

        for path in query_paths {
            for matcher in &matcher_array {
                if matcher.is_match(path) {
                    path_array.push(path.clone());
                    break;
                }
            }
        }
        path_array
    }
    pub fn get_matcher_array(&self) -> Vec<GlobMatcher> {
        let mut matcher_array: Vec<GlobMatcher> = vec![];
        let glob = if let Ok(glob) = Glob::new(self.path.as_str()) {
            glob
        } else {
            return matcher_array;
        };
        matcher_array.push(glob.compile_matcher());

        let paths_filter = match &self.paths_filter {
            Some(paths_filter) => paths_filter,
            None => return matcher_array,
        };

        // TODO: create a matcher in advance
        for path_filter in &paths_filter.paths {
            let regex = format!("{}.{}", self.path, path_filter);
            let glob = Glob::new(regex.as_str()).unwrap();
            matcher_array.push(glob.compile_matcher());
        }

        matcher_array
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

    // TODO: refactor
    pub fn check_change_filter(
        &self,
        previous_notified_value: &Option<Value>,
        current_value: &Value,
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

        let previous_notified_value = if let Some(previous_notified_value) = previous_notified_value
        {
            previous_notified_value
        } else {
            return true;
        };

        match previous_notified_value {
            Value::Bool(previous_notified_value) => {
                let Value::Bool(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                match change_filter.opecode {
                    Opecode::Equal => previous_notified_value == current_value,
                    Opecode::NotEqual => previous_notified_value != current_value,
                    Opecode::GreaterThan => !*previous_notified_value && *current_value,
                    Opecode::LesserThan => *previous_notified_value && !*current_value,
                    _ => true,
                }
            }
            Value::Int8(previous_notified_value) => {
                let Value::Int8(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                let current_value = *current_value;
                let previous_notified_value = *previous_notified_value;
                match change_filter.opecode {
                    Opecode::Equal => {
                        (current_value as i32 - change_filter.offset)
                            == previous_notified_value as i32
                    }
                    Opecode::NotEqual => {
                        (current_value as i32 - change_filter.offset)
                            != previous_notified_value as i32
                    }
                    Opecode::GreaterThan => {
                        (current_value as i32 - change_filter.offset)
                            > previous_notified_value as i32
                    }
                    Opecode::GreaterEqual => {
                        (current_value as i32 - change_filter.offset)
                            >= previous_notified_value as i32
                    }
                    Opecode::LesserThan => {
                        (current_value as i32 - change_filter.offset)
                            < previous_notified_value as i32
                    }
                    Opecode::LesserEqual => {
                        (current_value as i32 - change_filter.offset)
                            <= previous_notified_value as i32
                    }
                    _ => true,
                }
            }
            Value::Int16(previous_notified_value) => {
                let Value::Int16(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                let current_value = *current_value;
                let previous_notified_value = *previous_notified_value;
                match change_filter.opecode {
                    Opecode::Equal => {
                        (current_value as i32 - change_filter.offset)
                            == previous_notified_value as i32
                    }
                    Opecode::NotEqual => {
                        (current_value as i32 - change_filter.offset)
                            != previous_notified_value as i32
                    }
                    Opecode::GreaterThan => {
                        (current_value as i32 - change_filter.offset)
                            > previous_notified_value as i32
                    }
                    Opecode::GreaterEqual => {
                        (current_value as i32 - change_filter.offset)
                            >= previous_notified_value as i32
                    }
                    Opecode::LesserThan => {
                        (current_value as i32 - change_filter.offset)
                            < previous_notified_value as i32
                    }
                    Opecode::LesserEqual => {
                        (current_value as i32 - change_filter.offset)
                            <= previous_notified_value as i32
                    }
                    _ => true,
                }
            }
            Value::Int32(previous_notified_value) => {
                let Value::Int32(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                let current_value = *current_value;
                let previous_notified_value = *previous_notified_value;
                match change_filter.opecode {
                    Opecode::Equal => {
                        (current_value - change_filter.offset) == previous_notified_value
                    }
                    Opecode::NotEqual => {
                        (current_value - change_filter.offset) != previous_notified_value
                    }
                    Opecode::GreaterThan => {
                        (current_value - change_filter.offset) > previous_notified_value
                    }
                    Opecode::GreaterEqual => {
                        (current_value - change_filter.offset) >= previous_notified_value
                    }
                    Opecode::LesserThan => {
                        (current_value - change_filter.offset) < previous_notified_value
                    }
                    Opecode::LesserEqual => {
                        (current_value - change_filter.offset) <= previous_notified_value
                    }
                    _ => true,
                }
            }
            Value::Int64(previous_notified_value) => {
                let Value::Int64(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                let current_value = *current_value;
                let previous_notified_value = *previous_notified_value;
                match change_filter.opecode {
                    Opecode::Equal => {
                        (current_value - change_filter.offset as i64) == previous_notified_value
                    }
                    Opecode::NotEqual => {
                        (current_value - change_filter.offset as i64) != previous_notified_value
                    }
                    Opecode::GreaterThan => {
                        (current_value - change_filter.offset as i64) > previous_notified_value
                    }
                    Opecode::GreaterEqual => {
                        (current_value - change_filter.offset as i64) >= previous_notified_value
                    }
                    Opecode::LesserThan => {
                        (current_value - change_filter.offset as i64) < previous_notified_value
                    }
                    Opecode::LesserEqual => {
                        (current_value - change_filter.offset as i64) <= previous_notified_value
                    }
                    _ => true,
                }
            }
            Value::Uint8(previous_notified_value) => {
                let Value::Uint8(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                let current_value = *current_value;
                let previous_notified_value = *previous_notified_value;
                match change_filter.opecode {
                    Opecode::Equal => {
                        (current_value as i32 - change_filter.offset)
                            == previous_notified_value as i32
                    }
                    Opecode::NotEqual => {
                        (current_value as i32 - change_filter.offset)
                            != previous_notified_value as i32
                    }
                    Opecode::GreaterThan => {
                        (current_value as i32 - change_filter.offset)
                            > previous_notified_value as i32
                    }
                    Opecode::GreaterEqual => {
                        (current_value as i32 - change_filter.offset)
                            >= previous_notified_value as i32
                    }
                    Opecode::LesserThan => {
                        (current_value as i32 - change_filter.offset)
                            < previous_notified_value as i32
                    }
                    Opecode::LesserEqual => {
                        (current_value as i32 - change_filter.offset)
                            <= previous_notified_value as i32
                    }
                    _ => true,
                }
            }
            Value::Uint16(previous_notified_value) => {
                let Value::Uint16(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                let current_value = *current_value;
                let previous_notified_value = *previous_notified_value;
                match change_filter.opecode {
                    Opecode::Equal => {
                        (current_value as i32 - change_filter.offset)
                            == previous_notified_value as i32
                    }
                    Opecode::NotEqual => {
                        (current_value as i32 - change_filter.offset)
                            != previous_notified_value as i32
                    }
                    Opecode::GreaterThan => {
                        (current_value as i32 - change_filter.offset)
                            > previous_notified_value as i32
                    }
                    Opecode::GreaterEqual => {
                        (current_value as i32 - change_filter.offset)
                            >= previous_notified_value as i32
                    }
                    Opecode::LesserThan => {
                        (current_value as i32 - change_filter.offset)
                            < previous_notified_value as i32
                    }
                    Opecode::LesserEqual => {
                        (current_value as i32 - change_filter.offset)
                            <= previous_notified_value as i32
                    }
                    _ => true,
                }
            }
            Value::Uint32(previous_notified_value) => {
                let Value::Uint32(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                let current_value = *current_value;
                let previous_notified_value = *previous_notified_value;
                match change_filter.opecode {
                    Opecode::Equal => {
                        (current_value - (change_filter.offset as u32)) == previous_notified_value
                    }
                    Opecode::NotEqual => {
                        (current_value - (change_filter.offset as u32)) != previous_notified_value
                    }
                    Opecode::GreaterThan => {
                        (current_value - (change_filter.offset as u32)) > previous_notified_value
                    }
                    Opecode::GreaterEqual => {
                        (current_value - (change_filter.offset as u32)) >= previous_notified_value
                    }
                    Opecode::LesserThan => {
                        (current_value - (change_filter.offset as u32)) < previous_notified_value
                    }
                    Opecode::LesserEqual => {
                        (current_value - (change_filter.offset as u32)) <= previous_notified_value
                    }
                    _ => true,
                }
            }
            Value::Uint64(previous_notified_value) => {
                let Value::Uint64(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                let current_value = *current_value;
                let previous_notified_value = *previous_notified_value;
                match change_filter.opecode {
                    Opecode::Equal => {
                        (current_value - (change_filter.offset as u64)) == previous_notified_value
                    }
                    Opecode::NotEqual => {
                        (current_value - (change_filter.offset as u64)) != previous_notified_value
                    }
                    Opecode::GreaterThan => {
                        (current_value - (change_filter.offset as u64)) > previous_notified_value
                    }
                    Opecode::GreaterEqual => {
                        (current_value - (change_filter.offset as u64)) >= previous_notified_value
                    }
                    Opecode::LesserThan => {
                        (current_value - (change_filter.offset as u64)) < previous_notified_value
                    }
                    Opecode::LesserEqual => {
                        (current_value - (change_filter.offset as u64)) <= previous_notified_value
                    }
                    _ => true,
                }
            }
            Value::Float(previous_notified_value) => {
                let Value::Float(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                let current_value = *current_value;
                let previous_notified_value = *previous_notified_value;
                match change_filter.opecode {
                    Opecode::Equal => {
                        (current_value - (change_filter.offset as f32)) == previous_notified_value
                    }
                    Opecode::NotEqual => {
                        (current_value - (change_filter.offset as f32)) != previous_notified_value
                    }
                    Opecode::GreaterThan => {
                        (current_value - (change_filter.offset as f32)) > previous_notified_value
                    }
                    Opecode::GreaterEqual => {
                        (current_value - (change_filter.offset as f32)) >= previous_notified_value
                    }
                    Opecode::LesserThan => {
                        (current_value - (change_filter.offset as f32)) < previous_notified_value
                    }
                    Opecode::LesserEqual => {
                        (current_value - (change_filter.offset as f32)) <= previous_notified_value
                    }
                    _ => true,
                }
            }
            Value::Double(previous_notified_value) => {
                let Value::Double(current_value) = current_value else {
                    error!(
                        "previous value and current value are not same type: {}, {}",
                        previous_notified_value, current_value
                    );
                    return false;
                };

                let current_value = *current_value;
                let previous_notified_value = *previous_notified_value;
                match change_filter.opecode {
                    Opecode::Equal => {
                        (current_value - (change_filter.offset as f64)) == previous_notified_value
                    }
                    Opecode::NotEqual => {
                        (current_value - (change_filter.offset as f64)) != previous_notified_value
                    }
                    Opecode::GreaterThan => {
                        (current_value - (change_filter.offset as f64)) > previous_notified_value
                    }
                    Opecode::GreaterEqual => {
                        (current_value - (change_filter.offset as f64)) >= previous_notified_value
                    }
                    Opecode::LesserThan => {
                        (current_value - (change_filter.offset as f64)) < previous_notified_value
                    }
                    Opecode::LesserEqual => {
                        (current_value - (change_filter.offset as f64)) <= previous_notified_value
                    }
                    _ => true,
                }
            }
            _ => true,
        }
    }
}

impl GetCondition {
    pub fn get_matched_paths(&self, query_paths: &[String]) -> Vec<String> {
        let mut path_array: Vec<String> = vec![];
        let matcher_array: Vec<GlobMatcher> = self.get_matcher_array();

        for path in query_paths {
            for matcher in &matcher_array {
                if matcher.is_match(path) {
                    path_array.push(path.clone());
                    break;
                }
            }
        }
        path_array
    }
    pub fn get_matcher_array(&self) -> Vec<GlobMatcher> {
        let mut matcher_array: Vec<GlobMatcher> = vec![];
        let glob = if let Ok(glob) = Glob::new(self.path.as_str()) {
            glob
        } else {
            return matcher_array;
        };
        matcher_array.push(glob.compile_matcher());

        let paths_filter = match &self.paths_filter {
            Some(paths_filter) => paths_filter,
            None => return matcher_array,
        };

        for path_filter in &paths_filter.paths {
            let regex = format!("{}.{}", self.path, path_filter);
            let glob = Glob::new(regex.as_str()).unwrap();
            matcher_array.push(glob.compile_matcher());
        }

        matcher_array
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
    pub opecode: Opecode,
    pub boundary: i32,
}

#[derive(Clone, Debug)]
pub struct ChangeFilter {
    pub opecode: Opecode,
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
pub enum Opecode {
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
