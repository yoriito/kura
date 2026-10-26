#![no_main]
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


use kura::interface::subscription::{Mode, PathsFilter, SubscriptionCondition};
use libfuzzer_sys::fuzz_target;

fuzz_target!(|data: &[u8]| {
    let Ok(s) = std::str::from_utf8(data) else {
        return;
    };

    let condition = SubscriptionCondition {
        mode: Mode::Actual,
        path: "Vehicle.*".to_string(),
        paths_filter: Some(PathsFilter {
            paths: vec![s.to_string()],
        }),
        misc_filter: None,
    };

    let query_path = "Vehicle.Speed".to_string();

    // unwrap() calls in subscription.rs:61, 101 may panic on invalid glob patterns
    let _ = condition.is_match_path(&query_path);
    let _ = condition.get_matcher_array();
    let _ = condition.get_matched_paths(&[query_path]);
});
