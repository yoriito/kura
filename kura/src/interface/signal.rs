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

use bincode::{Decode, Encode};
use serde;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::fmt;
use std::fmt::Display;
use std::io;
use std::str::FromStr;

use strum_macros;

#[derive(Encode, Decode, serde::Serialize, Deserialize, Debug, Clone)]
pub struct Signal {
    pub path: String,
    pub current_datapoint: DataPoint,
    pub target_datapoint: DataPoint,
    pub config: Config,
}

#[derive(Encode, Decode, Serialize, Deserialize, Debug, Clone)]
pub struct Config {
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
    pub user_defined_items: Option<HashMap<String, String>>,
}

#[derive(Encode, Decode, Serialize, Deserialize, Clone, Debug)]
pub struct DataPoint {
    pub stored_timestamp: Option<i64>,
    pub produced_timestamp: Option<i64>,
    pub value: Value,
    pub is_available_sensor: Option<bool>,
    pub is_available_actuator: Option<bool>,
}

#[derive(Encode, Decode, Serialize, Deserialize, Clone, Debug)]
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
}

#[derive(Encode, Decode, Serialize, Deserialize, Clone, Debug)]
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
}

impl TryFrom<i32> for ValueType {
    type Error = KuraError;
    fn try_from(value: i32) -> Result<Self, KuraError> {
        match value {
            x if x == ValueType::TypeNAN as i32 => Ok(ValueType::TypeNAN),
            x if x == ValueType::TypeBool as i32 => Ok(ValueType::TypeBool),
            x if x == ValueType::TypeString as i32 => Ok(ValueType::TypeString),
            x if x == ValueType::TypeInt8 as i32 => Ok(ValueType::TypeInt8),
            x if x == ValueType::TypeInt16 as i32 => Ok(ValueType::TypeInt16),
            x if x == ValueType::TypeInt32 as i32 => Ok(ValueType::TypeInt32),
            x if x == ValueType::TypeInt64 as i32 => Ok(ValueType::TypeInt64),
            x if x == ValueType::TypeUint8 as i32 => Ok(ValueType::TypeUint8),
            x if x == ValueType::TypeUint16 as i32 => Ok(ValueType::TypeUint16),
            x if x == ValueType::TypeUint32 as i32 => Ok(ValueType::TypeUint32),
            x if x == ValueType::TypeUint64 as i32 => Ok(ValueType::TypeUint64),
            x if x == ValueType::TypeFloat as i32 => Ok(ValueType::TypeFloat),
            x if x == ValueType::TypeDouble as i32 => Ok(ValueType::TypeDouble),
            x if x == ValueType::TypeBoolArray as i32 => Ok(ValueType::TypeBoolArray),
            x if x == ValueType::TypeStringArray as i32 => Ok(ValueType::TypeStringArray),
            x if x == ValueType::TypeInt8Array as i32 => Ok(ValueType::TypeInt8Array),
            x if x == ValueType::TypeInt16Array as i32 => Ok(ValueType::TypeUint16Array),
            x if x == ValueType::TypeInt32Array as i32 => Ok(ValueType::TypeInt32Array),
            x if x == ValueType::TypeInt64Array as i32 => Ok(ValueType::TypeInt64Array),
            x if x == ValueType::TypeUint8Array as i32 => Ok(ValueType::TypeUint8Array),
            x if x == ValueType::TypeUint16Array as i32 => Ok(ValueType::TypeUint16Array),
            x if x == ValueType::TypeUint32Array as i32 => Ok(ValueType::TypeUint32Array),
            x if x == ValueType::TypeUint64Array as i32 => Ok(ValueType::TypeUint64Array),
            x if x == ValueType::TypeFloatArray as i32 => Ok(ValueType::TypeFloatArray),
            x if x == ValueType::TypeDoubleArray as i32 => Ok(ValueType::TypeDoubleArray),
            _ => Err(KuraError {
                number: KuraErrorCode::BadRequest,
                reason: KuraErrorReason::InvalidData,
                description: Some(format!("{value} is not valid value for ValueType")),
            }),
        }
    }
}

impl ValueType {
    pub fn is_match(&self, value: &Value) -> bool {
        match self {
            ValueType::TypeNAN => matches!(value, Value::Nan),
            ValueType::TypeBool => matches!(value, Value::Bool(_)),
            ValueType::TypeString => matches!(value, Value::String(_)),
            ValueType::TypeInt8 => matches!(value, Value::Int8(_)),
            ValueType::TypeInt16 => matches!(value, Value::Int16(_)),
            ValueType::TypeInt32 => matches!(value, Value::Int32(_)),
            ValueType::TypeInt64 => matches!(value, Value::Int64(_)),
            ValueType::TypeUint8 => matches!(value, Value::Uint8(_)),
            ValueType::TypeUint16 => matches!(value, Value::Uint16(_)),
            ValueType::TypeUint32 => matches!(value, Value::Uint32(_)),
            ValueType::TypeUint64 => matches!(value, Value::Uint64(_)),
            ValueType::TypeFloat => matches!(value, Value::Float(_)),
            ValueType::TypeDouble => matches!(value, Value::Double(_)),
            ValueType::TypeBoolArray => matches!(value, Value::BoolArray(_)),
            ValueType::TypeStringArray => matches!(value, Value::StringArray(_)),
            ValueType::TypeInt8Array => matches!(value, Value::Int8Array(_)),
            ValueType::TypeInt16Array => matches!(value, Value::Int16Array(_)),
            ValueType::TypeInt32Array => matches!(value, Value::Int32Array(_)),
            ValueType::TypeInt64Array => matches!(value, Value::Int64Array(_)),
            ValueType::TypeUint8Array => matches!(value, Value::Uint8Array(_)),
            ValueType::TypeUint16Array => matches!(value, Value::Uint16Array(_)),
            ValueType::TypeUint32Array => matches!(value, Value::Uint32Array(_)),
            ValueType::TypeUint64Array => matches!(value, Value::Uint64Array(_)),
            ValueType::TypeFloatArray => matches!(value, Value::FloatArray(_)),
            ValueType::TypeDoubleArray => matches!(value, Value::DoubleArray(_)),
        }
    }
}

impl fmt::Display for ValueType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        let data_type_str = match self {
            ValueType::TypeNAN => "NAN".to_string(),
            ValueType::TypeBool => "boolean".to_string(),
            ValueType::TypeString => "string".to_string(),
            ValueType::TypeInt8 => "int8".to_string(),
            ValueType::TypeInt16 => "int16".to_string(),
            ValueType::TypeInt32 => "int32".to_string(),
            ValueType::TypeInt64 => "int64".to_string(),
            ValueType::TypeUint8 => "uint8".to_string(),
            ValueType::TypeUint16 => "uint16".to_string(),
            ValueType::TypeUint32 => "uint32".to_string(),
            ValueType::TypeUint64 => "uint64".to_string(),
            ValueType::TypeFloat => "float".to_string(),
            ValueType::TypeDouble => "double".to_string(),
            ValueType::TypeBoolArray => "boolean[]".to_string(),
            ValueType::TypeStringArray => "string[]".to_string(),
            ValueType::TypeInt8Array => "int8[]".to_string(),
            ValueType::TypeInt16Array => "int16[]".to_string(),
            ValueType::TypeInt32Array => "int32[]".to_string(),
            ValueType::TypeInt64Array => "int64[]".to_string(),
            ValueType::TypeUint8Array => "uint8[]".to_string(),
            ValueType::TypeUint16Array => "uint16[]".to_string(),
            ValueType::TypeUint32Array => "uint32[]".to_string(),
            ValueType::TypeUint64Array => "uint64[]".to_string(),
            ValueType::TypeFloatArray => "float[]".to_string(),
            ValueType::TypeDoubleArray => "double[]".to_string(),
        };
        write!(f, "{data_type_str}")
    }
}

impl FromStr for ValueType {
    type Err = Box<dyn std::error::Error>;
    fn from_str(s: &str) -> Result<Self, Box<dyn std::error::Error>> {
        match s.to_lowercase().as_str() {
            "boolean" => Ok(ValueType::TypeBool),
            "string" => Ok(ValueType::TypeString),
            "int8" => Ok(ValueType::TypeInt8),
            "int16" => Ok(ValueType::TypeInt16),
            "int32" => Ok(ValueType::TypeInt32),
            "int64" => Ok(ValueType::TypeInt64),
            "uint8" => Ok(ValueType::TypeUint8),
            "uint16" => Ok(ValueType::TypeUint16),
            "uint32" => Ok(ValueType::TypeUint32),
            "uint64" => Ok(ValueType::TypeUint64),
            "float" => Ok(ValueType::TypeFloat),
            "double" => Ok(ValueType::TypeDouble),
            "boolean[]" => Ok(ValueType::TypeBoolArray),
            "string[]" => Ok(ValueType::TypeStringArray),
            "int8[]" => Ok(ValueType::TypeInt8Array),
            "int16[]" => Ok(ValueType::TypeInt16Array),
            "int32[]" => Ok(ValueType::TypeInt32Array),
            "int64[]" => Ok(ValueType::TypeInt64Array),
            "uint8[]" => Ok(ValueType::TypeUint8Array),
            "uint16[]" => Ok(ValueType::TypeUint16Array),
            "uint32[]" => Ok(ValueType::TypeUint32Array),
            "uint64[]" => Ok(ValueType::TypeUint64Array),
            "float[]" => Ok(ValueType::TypeFloatArray),
            "double[]" => Ok(ValueType::TypeDoubleArray),
            _ => Err(Box::new(io::Error::new(io::ErrorKind::InvalidData, s))),
        }
    }
}

#[derive(Encode, Decode, Serialize, Deserialize, Debug, Clone, PartialEq)]
#[repr(i32)]
pub enum LeafType {
    Branch = 0,
    Sensor = 1,
    Attribute = 2,
    Actuator = 3,
}

impl TryFrom<i32> for LeafType {
    type Error = KuraError;

    fn try_from(v: i32) -> Result<Self, Self::Error> {
        match v {
            x if x == LeafType::Branch as i32 => Ok(LeafType::Branch),
            x if x == LeafType::Sensor as i32 => Ok(LeafType::Sensor),
            x if x == LeafType::Attribute as i32 => Ok(LeafType::Attribute),
            x if x == LeafType::Actuator as i32 => Ok(LeafType::Actuator),
            _ => Err(KuraError {
                number: KuraErrorCode::BadRequest,
                reason: KuraErrorReason::InvalidData,
                description: Some(format!("{v} is invalied LeafType value.")),
            }),
        }
    }
}

impl FromStr for LeafType {
    type Err = Box<dyn std::error::Error>;
    fn from_str(s: &str) -> Result<Self, Box<dyn std::error::Error>> {
        match s.to_lowercase().as_str() {
            "branch" => Ok(LeafType::Branch),
            "sensor" => Ok(LeafType::Sensor),
            "attribute" => Ok(LeafType::Attribute),
            "actuator" => Ok(LeafType::Actuator),
            _ => Err(Box::new(io::Error::new(io::ErrorKind::InvalidData, s))),
        }
    }
}

#[derive(
    Clone,
    Copy,
    Debug,
    PartialEq,
    Eq,
    Hash,
    PartialOrd,
    Ord,
    ::prost::Enumeration,
    strum_macros::Display,
)]
#[repr(i32)]
pub enum KuraErrorCode {
    /// Unspecified error code
    Unspecified = 0,
    /// error when the request is invalid or malformed
    BadRequest = 400,
    /// error when the request is unauthorized
    Unauthorized = 401,
    /// error when the request is forbidden
    Forbidden = 403,
    /// error when the requested node is not found
    NotFound = 404,
    /// error when the request is not acceptable
    RequestTimeout = 408,
    /// error when the server is overloaded or too many requests are made
    TooManyRequests = 429,
    /// error when the server encountered an internal error
    BadGateway = 502,
    /// error when the server is currently unavailable
    ServiceUnavailable = 503,
    /// error when the server did not receive a timely response from an upstream server
    GatewayTimeout = 504,
}

#[derive(
    Clone,
    Copy,
    Debug,
    PartialEq,
    Eq,
    Hash,
    PartialOrd,
    Ord,
    ::prost::Enumeration,
    strum_macros::Display,
)]
#[repr(i32)]
pub enum KuraErrorReason {
    /// Unspecified reason for the error
    Unspecified = 0,
    /// Bad request, typically due to malformed request syntax
    BadRequest = 1,
    /// Invalid data provided in the request
    InvalidData = 2,
    /// Invalid token, such as an expired or malformed request id
    InvalidToken = 3,
    /// Forbidden request, such as when the user does not have permission to access the resource
    ForbiddenRequest = 4,
    /// Unavailable data, such as when the requested data is not available or cannot be retrieved
    UnavailableData = 5,
    /// Request timeout, such as when the server did not receive a response in time
    RequestTimeout = 6,
    /// Too many requests, such as when the server is overloaded or too many requests are made in a short period
    TooManyRequests = 7,
    /// Bad gateway, such as when the server encountered an error while communicating with an upstream server
    BadGateway = 8,
    /// Service unavailable, such as when the server is currently unavailable or undergoing maintenance
    ServiceUnavailable = 9,
    /// Gateway timeout, such as when the server did not receive a timely response from an upstream server
    GatewayTimeout = 10,
}

#[derive(Clone, PartialEq)]
pub struct KuraError {
    pub number: KuraErrorCode,
    pub reason: KuraErrorReason,
    pub description: Option<String>,
}

pub type KuraResult<T> = std::result::Result<T, KuraError>;

//pub fn build_kura_error_result<T>(
//    number: KuraErrorCode,
//    reason: KuraErrorReason,
//    description: Option<String> ) -> KuraResult<T> {
//    KuraResult::Err(KuraError { number, reason, description })
//}

pub trait MapExternalToKuraResult<T, E> {
    fn to_kura_result(
        self,
        err_code: KuraErrorCode,
        err_reason: KuraErrorReason,
        description: String,
    ) -> KuraResult<T>;
}

impl<T, E: std::fmt::Debug> MapExternalToKuraResult<T, E> for std::result::Result<T, E> {
    fn to_kura_result(
        self,
        err_code: KuraErrorCode,
        err_reason: KuraErrorReason,
        description: String,
    ) -> KuraResult<T> {
        self.map_err(|error| KuraError {
            number: err_code,
            reason: err_reason,
            description: Some(format!("{description}: {error:?}")),
        })
    }
}

pub trait MapOptionToKuraError<T> {
    fn to_kura_result(
        self,
        err_code: KuraErrorCode,
        err_reason: KuraErrorReason,
        description: String,
    ) -> KuraResult<T>;
}

impl<T> MapOptionToKuraError<T> for Option<T> {
    fn to_kura_result(
        self,
        err_code: KuraErrorCode,
        err_reason: KuraErrorReason,
        description: String,
    ) -> KuraResult<T> {
        match self {
            Some(value) => Ok(value),
            None => Err(KuraError {
                number: err_code,
                reason: err_reason,
                description: Some(description),
            }),
        }
    }
}

//------------------------------------------------------------------------------------------------

impl Display for Signal {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{{ \"path\": \"{}\", \"value\": {}, \"config\": {} }}",
            self.path, self.target_datapoint, self.config
        )
    }
}

impl Display for DataPoint {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{{ \"stored_timestamp\": {},  \"produced_timestamp\": {}, \"value\": {}, }}",
            self.stored_timestamp.unwrap_or(-1),
            self.produced_timestamp.unwrap_or(-1),
            self.value,
        )
    }
}

impl Display for Value {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Value::Nan => write!(f, "None"),
            Value::Bool(v) => write!(f, "{v}"),
            Value::String(v) => write!(f, "{v}"),
            Value::Int8(v) => write!(f, "{v}"),
            Value::Int16(v) => write!(f, "{v}"),
            Value::Int32(v) => write!(f, "{v}"),
            Value::Int64(v) => write!(f, "{v}"),
            Value::Uint8(v) => write!(f, "{v}"),
            Value::Uint16(v) => write!(f, "{v}"),
            Value::Uint32(v) => write!(f, "{v}"),
            Value::Uint64(v) => write!(f, "{v}"),
            Value::Float(v) => write!(f, "{v}"),
            Value::Double(v) => write!(f, "{v}"),
            Value::BoolArray(v) => write!(f, "{v:?}"),
            Value::StringArray(v) => write!(f, "{v:?}"),
            Value::Int8Array(v) => write!(f, "{v:?}"),
            Value::Int16Array(v) => write!(f, "{v:?}"),
            Value::Int32Array(v) => write!(f, "{v:?}"),
            Value::Int64Array(v) => write!(f, "{v:?}"),
            Value::Uint8Array(v) => write!(f, "{v:?}"),
            Value::Uint16Array(v) => write!(f, "{v:?}"),
            Value::Uint32Array(v) => write!(f, "{v:?}"),
            Value::Uint64Array(v) => write!(f, "{v:?}"),
            Value::FloatArray(v) => write!(f, "{v:?}"),
            Value::DoubleArray(v) => write!(f, "{v:?}"),
        }
    }
}

impl Display for Config {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{{ \
            \"leaf_type\": \"{}\", \
            \"deprecation\": \"{:?}\", \
            \"unit\": \"{:?}\", \
            \"min\": \"{:?}\", \
            \"max\": \"{:?}\", \
            \"description\": \"{:?}\", \
            \"comment\": \"{:?}\", \
            \"allowd\": \"{:?}\", \
            \"default\": \"{:?}\" }}",
            self.leaf_type,
            self.deprecation,
            self.unit,
            self.min,
            self.max,
            self.description,
            self.comment,
            self.allowd,
            self.default,
        )
    }
}

impl Display for LeafType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let name = match self {
            LeafType::Branch => String::from("Branch"),
            LeafType::Sensor => String::from("Sensor"),
            LeafType::Attribute => String::from("Attribute"),
            LeafType::Actuator => String::from("Actuator"),
        };
        write!(f, "{name}")
    }
}

impl Display for KuraError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let description = match self.description.clone() {
            Some(description) => description,
            None => "-".to_string(),
        };
        write!(
            f,
            "ErrorCode: {}, ErrorReason: {}, Detaild: {}",
            self.number, self.reason, description
        )
    }
}
