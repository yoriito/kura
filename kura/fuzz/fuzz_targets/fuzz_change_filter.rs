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


use kura::interface::model::{SignalState, Value};
use kura::interface::subscription::{ChangeFilter, MiscFilter, Mode, Opcode, SubscriptionCondition};
use libfuzzer_sys::arbitrary;
use libfuzzer_sys::arbitrary::Arbitrary;
use libfuzzer_sys::fuzz_target;

#[derive(Debug, Arbitrary)]
enum FuzzOpcode {
    Undefined,
    Equal,
    NotEqual,
    GreaterThan,
    GreaterEqual,
    LesserThan,
    LesserEqual,
}

#[derive(Debug, Arbitrary)]
struct FuzzInput {
    opcode: FuzzOpcode,
    offset: i32,
    u32_prev: u32,
    u32_curr: u32,
    u64_prev: u64,
    u64_curr: u64,
    i32_prev: i32,
    i32_curr: i32,
    f32_prev: f32,
    f32_curr: f32,
}

fn make_opcode(op: &FuzzOpcode) -> Opcode {
    match op {
        FuzzOpcode::Undefined => Opcode::Undefined,
        FuzzOpcode::Equal => Opcode::Equal,
        FuzzOpcode::NotEqual => Opcode::NotEqual,
        FuzzOpcode::GreaterThan => Opcode::GreaterThan,
        FuzzOpcode::GreaterEqual => Opcode::GreaterEqual,
        FuzzOpcode::LesserThan => Opcode::LesserThan,
        FuzzOpcode::LesserEqual => Opcode::LesserEqual,
    }
}

fn make_state(value: Value) -> SignalState {
    SignalState {
        stored_timestamp: None,
        produced_timestamp: None,
        value,
        is_available_sensor: None,
        is_available_actuator: None,
    }
}

fuzz_target!(|input: FuzzInput| {
    let condition = SubscriptionCondition {
        mode: Mode::Actual,
        path: "Vehicle.*".to_string(),
        paths_filter: None,
        misc_filter: Some(MiscFilter::Change(ChangeFilter {
            opcode: make_opcode(&input.opcode),
            offset: input.offset,
        })),
    };

    // Uint32: subtraction may underflow if offset > current_value
    let _ = condition.check_change_filter(
        &Some(make_state(Value::Uint32(input.u32_prev))),
        &make_state(Value::Uint32(input.u32_curr)),
    );

    // Uint64: subtraction may underflow if offset > current_value
    let _ = condition.check_change_filter(
        &Some(make_state(Value::Uint64(input.u64_prev))),
        &make_state(Value::Uint64(input.u64_curr)),
    );

    // Int32: subtraction may overflow with extreme values
    let _ = condition.check_change_filter(
        &Some(make_state(Value::Int32(input.i32_prev))),
        &make_state(Value::Int32(input.i32_curr)),
    );

    // Float: comparison with NaN / Inf
    let _ = condition.check_change_filter(
        &Some(make_state(Value::Float(input.f32_prev))),
        &make_state(Value::Float(input.f32_curr)),
    );
});
