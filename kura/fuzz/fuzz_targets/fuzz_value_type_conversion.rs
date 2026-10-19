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


use kura::interface::{model::{Value, ValueType}, result};
use libfuzzer_sys::fuzz_target;

fn value_type_try_from_i32(value: i32) -> Result<ValueType, result::KuraError> {
    match value {
        0 => Ok(ValueType::TypeNAN),
        1 => Ok(ValueType::TypeBool),
        2 => Ok(ValueType::TypeString),
        3 => Ok(ValueType::TypeInt8),
        4 => Ok(ValueType::TypeInt16),
        5 => Ok(ValueType::TypeInt32),
        6 => Ok(ValueType::TypeInt64),
        7 => Ok(ValueType::TypeUint8),
        8 => Ok(ValueType::TypeUint16),
        9 => Ok(ValueType::TypeUint32),
        10 => Ok(ValueType::TypeUint64),
        11 => Ok(ValueType::TypeFloat),
        12 => Ok(ValueType::TypeDouble),
        13 => Ok(ValueType::TypeBoolArray),
        14 => Ok(ValueType::TypeStringArray),
        15 => Ok(ValueType::TypeInt8Array),
        16 => Ok(ValueType::TypeInt16Array),
        17 => Ok(ValueType::TypeInt32Array),
        18 => Ok(ValueType::TypeInt64Array),
        19 => Ok(ValueType::TypeUint8Array),
        20 => Ok(ValueType::TypeUint16Array),
        21 => Ok(ValueType::TypeUint32Array),
        22 => Ok(ValueType::TypeUint64Array),
        23 => Ok(ValueType::TypeFloatArray),
        24 => Ok(ValueType::TypeDoubleArray),
        _ => Err(result::KuraError {
            number: result::KuraErrorCode::BadRequest,
            reason: result::KuraErrorReason::InvalidData,
            description: Some(format!("{value} is not valid value for ValueType")),
        }),
    }
}

// Returns the expected Value for a given discriminant, independently of try_from().
// By deriving the expected value directly from the discriminant rather than from
// try_from()'s return value, a wrong variant returned by try_from() is detected
// as an is_match() mismatch.
fn expected_value(discriminant: i32) -> Option<Value> {
    match discriminant {
        0 => Some(Value::Nan),
        1 => Some(Value::Bool(false)),
        2 => Some(Value::String(String::new())),
        3 => Some(Value::Int8(0)),
        4 => Some(Value::Int16(0)),
        5 => Some(Value::Int32(0)),
        6 => Some(Value::Int64(0)),
        7 => Some(Value::Uint8(0)),
        8 => Some(Value::Uint16(0)),
        9 => Some(Value::Uint32(0)),
        10 => Some(Value::Uint64(0)),
        11 => Some(Value::Float(0.0)),
        12 => Some(Value::Double(0.0)),
        13 => Some(Value::BoolArray(vec![])),
        14 => Some(Value::StringArray(vec![])),
        15 => Some(Value::Int8Array(vec![])),
        16 => Some(Value::Int16Array(vec![])),
        17 => Some(Value::Int32Array(vec![])),
        18 => Some(Value::Int64Array(vec![])),
        19 => Some(Value::Uint8Array(vec![])),
        20 => Some(Value::Uint16Array(vec![])),
        21 => Some(Value::Uint32Array(vec![])),
        22 => Some(Value::Uint64Array(vec![])),
        23 => Some(Value::FloatArray(vec![])),
        24 => Some(Value::DoubleArray(vec![])),
        _ => None,
    }
}

fuzz_target!(|data: &[u8]| {
    if data.len() < 4 {
        return;
    }
    let value_type_int = i32::from_le_bytes([data[0], data[1], data[2], data[3]]);

    let Ok(value_type) = value_type_try_from_i32(value_type_int) else {
        return;
    };

    // Retrieve the expected Value from the discriminant.
    // If try_from() returned Ok for an invalid discriminant, expected_value() returns None
    // and the panic below catches it as a bug.
    let Some(representative_value) = expected_value(value_type_int) else {
        panic!(
            "ValueType::try_from({value_type_int}) returned Ok({value_type:?}), \
             but {value_type_int} is not a valid discriminant"
        );
    };

    // If try_from() returns the correct variant, is_match() must always be true.
    // A wrong conversion such as TypeInt16Array(16) -> TypeUint16Array would cause
    // TypeUint16Array.is_match(&Value::Int16Array([])) == false, which is caught here.
    assert!(
        value_type.is_match(&representative_value),
        "ValueType::try_from({value_type_int}) returned {value_type:?}, \
         but is_match() returned false for the representative Value"
    );
});
