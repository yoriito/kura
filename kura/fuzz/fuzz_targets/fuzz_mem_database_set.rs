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


use kura::entity::mem_database::MemDatabase;
use kura::interface::model::{LeafType, SignalMetadata, SignalState, Value, ValueType};
use kura::interface::traits::Database;
use libfuzzer_sys::arbitrary;
use libfuzzer_sys::arbitrary::Arbitrary;
use libfuzzer_sys::fuzz_target;
use std::sync::OnceLock;
use tokio::runtime::Runtime;

static RT: OnceLock<Runtime> = OnceLock::new();

#[derive(Debug, Arbitrary)]
struct FuzzInput {
    // Value variant index: 0=Nan, 1=Bool, 2=String, ..., 24=DoubleArray
    value_variant: u8,
    bool_value: bool,
    i8_value: i8,
    i16_value: i16,
    i32_value: i32,
    i64_value: i64,
    u8_value: u8,
    u16_value: u16,
    u32_value: u32,
    u64_value: u64,
    f32_value: f32,
    f64_value: f64,
    string_value: String,
    // ValueType index: 0=TypeNAN, ..., 24=TypeDoubleArray
    value_type_index: u8,
}

fn make_value(input: &FuzzInput) -> Value {
    match input.value_variant % 25 {
        0 => Value::Nan,
        1 => Value::Bool(input.bool_value),
        2 => Value::String(input.string_value.clone()),
        3 => Value::Int8(input.i8_value),
        4 => Value::Int16(input.i16_value),
        5 => Value::Int32(input.i32_value),
        6 => Value::Int64(input.i64_value),
        7 => Value::Uint8(input.u8_value),
        8 => Value::Uint16(input.u16_value),
        9 => Value::Uint32(input.u32_value),
        10 => Value::Uint64(input.u64_value),
        11 => Value::Float(input.f32_value),
        12 => Value::Double(input.f64_value),
        13 => Value::BoolArray(vec![input.bool_value]),
        14 => Value::StringArray(vec![input.string_value.clone()]),
        15 => Value::Int8Array(vec![input.i8_value]),
        16 => Value::Int16Array(vec![input.i16_value]),
        17 => Value::Int32Array(vec![input.i32_value]),
        18 => Value::Int64Array(vec![input.i64_value]),
        19 => Value::Uint8Array(vec![input.u8_value]),
        20 => Value::Uint16Array(vec![input.u16_value]),
        21 => Value::Uint32Array(vec![input.u32_value]),
        22 => Value::Uint64Array(vec![input.u64_value]),
        23 => Value::FloatArray(vec![input.f32_value]),
        _ => Value::DoubleArray(vec![input.f64_value]),
    }
}

fn make_value_type(input: &FuzzInput) -> ValueType {
    match input.value_type_index % 25 {
        0 => ValueType::TypeNAN,
        1 => ValueType::TypeBool,
        2 => ValueType::TypeString,
        3 => ValueType::TypeInt8,
        4 => ValueType::TypeInt16,
        5 => ValueType::TypeInt32,
        6 => ValueType::TypeInt64,
        7 => ValueType::TypeUint8,
        8 => ValueType::TypeUint16,
        9 => ValueType::TypeUint32,
        10 => ValueType::TypeUint64,
        11 => ValueType::TypeFloat,
        12 => ValueType::TypeDouble,
        13 => ValueType::TypeBoolArray,
        14 => ValueType::TypeStringArray,
        15 => ValueType::TypeInt8Array,
        16 => ValueType::TypeInt16Array,
        17 => ValueType::TypeInt32Array,
        18 => ValueType::TypeInt64Array,
        19 => ValueType::TypeUint8Array,
        20 => ValueType::TypeUint16Array,
        21 => ValueType::TypeUint32Array,
        22 => ValueType::TypeUint64Array,
        23 => ValueType::TypeFloatArray,
        _ => ValueType::TypeDoubleArray,
    }
}

fuzz_target!(|input: FuzzInput| {
    // Initialize the runtime once per process lifetime; creating it per iteration is too expensive.
    let rt = RT.get_or_init(|| Runtime::new().unwrap());

    rt.block_on(async {
        // Create a fresh MemDatabase each iteration to reset state.
        let Ok(db) = MemDatabase::create() else {
            return;
        };

        let value_type = make_value_type(&input);
        let value = make_value(&input);
        let path = "Vehicle.Speed".to_string();

        let metadata = SignalMetadata {
            leaf_type: LeafType::Sensor,
            data_type: value_type,
            deprecation: None,
            unit: None,
            min: None,
            max: None,
            description: None,
            comment: None,
            allowd: None,
            default: None,
            array_size: None,
            pattern: None,
            user_defined_items: None,
        };

        let _ = db.initialize_signal(path.clone(), metadata).await;

        let state = SignalState {
            stored_timestamp: None,
            produced_timestamp: None,
            value,
            is_available_sensor: None,
            is_available_actuator: None,
        };
        let _ = db.set_actual_state(path, state).await;
    });
});
