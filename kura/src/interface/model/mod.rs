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

pub mod converter;
pub mod display;

use bincode::{Decode, Encode};
use serde;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

#[derive(Encode, Decode, serde::Serialize, Deserialize, Debug, Clone, PartialEq)]
pub struct SignalMetadata {
    pub leaf_type: LeafType,
    pub data_type: ValueType,
    pub deprecation: Option<String>,
    pub unit: Option<String>,
    pub min: Option<Value>,
    pub max: Option<Value>,
    pub description: Option<String>,
    pub comment: Option<String>,
    pub allowd: Option<Vec<String>>,
    pub default: Option<Value>,
    pub array_size: Option<u64>,
    pub pattern: Option<Vec<String>>,
    pub user_defined_items: Option<HashMap<String, String>>,
}

//VSSUserDefinedType

#[derive(Encode, Decode, serde::Serialize, Deserialize, Debug, Clone, PartialEq)]
pub struct VSSUserDefinedType {
    pub name: String,
    pub description: Option<String>,
    pub deprecation: Option<String>,
    pub comment: Option<String>,
    pub children: HashMap<String, PropertyMetadata>,
}

#[derive(Encode, Decode, serde::Serialize, Deserialize, Debug, Clone, PartialEq)]
pub struct PropertyMetadata {
    pub leaf_type: LeafType,
    pub data_type: ValueType,
    pub deprecation: Option<String>,
    pub unit: Option<String>,
    pub pattern: Option<Vec<String>>,
    pub min: Option<Value>,
    pub max: Option<Value>,
    pub description: Option<String>,
    pub comment: Option<String>,
    pub allowd: Option<Vec<String>>,
    pub default: Option<Value>,
    pub array_size: Option<u64>,
    pub user_defined_items: Option<HashMap<String, String>>,
}

#[derive(Encode, Decode, serde::Serialize, Deserialize, Debug, Clone, PartialEq)]
pub struct SignalState {
    pub stored_timestamp: Option<i64>,
    pub produced_timestamp: Option<i64>,
    pub value: Value,
    pub is_available_sensor: Option<bool>,
    pub is_available_actuator: Option<bool>,
}

#[derive(Encode, Decode, Serialize, Deserialize, Clone, Debug, PartialEq)]
pub enum Value {
    Nan,
    Bool(bool),
    String(String),
    Int8(i8),
    Int16(i16),
    Int32(i32),
    Int64(i64),
    Uint8(u8),
    Uint16(u16),
    Uint32(u32),
    Uint64(u64),
    Float(f32),
    Double(f64),
    BoolArray(Vec<bool>),
    StringArray(Vec<String>),
    Int8Array(Vec<i8>),
    Int16Array(Vec<i16>),
    Int32Array(Vec<i32>),
    Int64Array(Vec<i64>),
    Uint8Array(Vec<u8>),
    Uint16Array(Vec<u16>),
    Uint32Array(Vec<u32>),
    Uint64Array(Vec<u64>),
    FloatArray(Vec<f32>),
    DoubleArray(Vec<f64>),
    Complex(HashMap<String, Value>),
    ComplexArray(Vec<HashMap<String, Value>>),
}

#[derive(Encode, Decode, Serialize, Deserialize, Clone, Debug, PartialEq)]
pub enum ValueType {
    TypeNAN,
    TypeBool,
    TypeString,
    TypeInt8,
    TypeInt16,
    TypeInt32,
    TypeInt64,
    TypeUint8,
    TypeUint16,
    TypeUint32,
    TypeUint64,
    TypeFloat,
    TypeDouble,
    TypeBoolArray,
    TypeStringArray,
    TypeInt8Array,
    TypeInt16Array,
    TypeInt32Array,
    TypeInt64Array,
    TypeUint8Array,
    TypeUint16Array,
    TypeUint32Array,
    TypeUint64Array,
    TypeFloatArray,
    TypeDoubleArray,
    TypeComplex(String),
    TypeComplexArray(String),
}

#[derive(Encode, Decode, Serialize, Deserialize, Debug, Clone, PartialEq)]
#[repr(i32)]
pub enum LeafType {
    Branch = 0,
    Sensor = 1,
    Attribute = 2,
    Actuator = 3,
    Property = 4,
}

#[derive(Encode, Decode, Serialize, Deserialize, Debug, Clone, PartialEq)]
pub struct SignalStateView {
    pub path: String,
    pub state: SignalState,
}

#[derive(Encode, Decode, Serialize, Deserialize, Debug, Clone, PartialEq)]
pub struct SignalMetadataView {
    pub path: String,
    pub metadata: SignalMetadata,
}
