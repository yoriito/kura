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

use crate::interface::model;
use crate::interface::result;
use std::io;
use std::str::FromStr;

use crate::controller::yoriito::common::viss as common_viss;

impl TryFrom<(Option<String>, common_viss::DataType)> for model::ValueType {
    type Error = result::KuraError;
    fn try_from(tuple: (Option<String>, common_viss::DataType)) -> Result<Self, result::KuraError> {
        let (name, value) = tuple;
        match value {
            common_viss::DataType::DatatypeNone => Ok(model::ValueType::TypeNAN),
            common_viss::DataType::DatatypeString => Ok(model::ValueType::TypeString),
            common_viss::DataType::DatatypeBool => Ok(model::ValueType::TypeBool),
            common_viss::DataType::DatatypeInt8 => Ok(model::ValueType::TypeInt8),
            common_viss::DataType::DatatypeInt16 => Ok(model::ValueType::TypeInt16),
            common_viss::DataType::DatatypeInt32 => Ok(model::ValueType::TypeInt32),
            common_viss::DataType::DatatypeInt64 => Ok(model::ValueType::TypeInt64),
            common_viss::DataType::DatatypeUint8 => Ok(model::ValueType::TypeUint8),
            common_viss::DataType::DatatypeUint16 => Ok(model::ValueType::TypeUint16),
            common_viss::DataType::DatatypeUint32 => Ok(model::ValueType::TypeUint32),
            common_viss::DataType::DatatypeUint64 => Ok(model::ValueType::TypeUint64),
            common_viss::DataType::DatatypeFloat => Ok(model::ValueType::TypeFloat),
            common_viss::DataType::DatatypeDouble => Ok(model::ValueType::TypeDouble),
            common_viss::DataType::DatatypeArrayString => Ok(model::ValueType::TypeStringArray),
            common_viss::DataType::DatatypeArrayBool => Ok(model::ValueType::TypeBoolArray),
            common_viss::DataType::DatatypeArrayInt8 => Ok(model::ValueType::TypeInt8Array),
            common_viss::DataType::DatatypeArrayInt16 => Ok(model::ValueType::TypeInt16Array),
            common_viss::DataType::DatatypeArrayInt32 => Ok(model::ValueType::TypeInt32Array),
            common_viss::DataType::DatatypeArrayInt64 => Ok(model::ValueType::TypeInt64Array),
            common_viss::DataType::DatatypeArrayUint8 => Ok(model::ValueType::TypeUint8Array),
            common_viss::DataType::DatatypeArrayUint16 => Ok(model::ValueType::TypeUint16Array),
            common_viss::DataType::DatatypeArrayUint32 => Ok(model::ValueType::TypeUint32Array),
            common_viss::DataType::DatatypeArrayUint64 => Ok(model::ValueType::TypeUint64Array),
            common_viss::DataType::DatatypeArrayFloat => Ok(model::ValueType::TypeFloatArray),
            common_viss::DataType::DatatypeArrayDouble => Ok(model::ValueType::TypeDoubleArray),
            common_viss::DataType::DatatypeStruct => {
                let name = if let Some(name) = name {
                    name
                } else {
                    return result::KuraResult::Err(result::KuraError {
                        number: result::KuraErrorCode::BadRequest,
                        reason: result::KuraErrorReason::InvalidData,
                        description: Some("Anonymous struct".to_string()),
                    });
                };

                Ok(model::ValueType::TypeComplex(name))
            }
            common_viss::DataType::DatatypeStructArray => {
                let name = if let Some(name) = name {
                    name
                } else {
                    return result::KuraResult::Err(result::KuraError {
                        number: result::KuraErrorCode::BadRequest,
                        reason: result::KuraErrorReason::InvalidData,
                        description: Some("Anonymous struct array".to_string()),
                    });
                };

                Ok(model::ValueType::TypeComplexArray(name))
            }
        }
    }
}

impl model::ValueType {
    pub fn is_match(&self, value: &model::Value) -> bool {
        match self {
            model::ValueType::TypeNAN => matches!(value, model::Value::Nan),
            model::ValueType::TypeBool => matches!(value, model::Value::Bool(_)),
            model::ValueType::TypeString => matches!(value, model::Value::String(_)),
            model::ValueType::TypeInt8 => matches!(value, model::Value::Int8(_)),
            model::ValueType::TypeInt16 => matches!(value, model::Value::Int16(_)),
            model::ValueType::TypeInt32 => matches!(value, model::Value::Int32(_)),
            model::ValueType::TypeInt64 => matches!(value, model::Value::Int64(_)),
            model::ValueType::TypeUint8 => matches!(value, model::Value::Uint8(_)),
            model::ValueType::TypeUint16 => matches!(value, model::Value::Uint16(_)),
            model::ValueType::TypeUint32 => matches!(value, model::Value::Uint32(_)),
            model::ValueType::TypeUint64 => matches!(value, model::Value::Uint64(_)),
            model::ValueType::TypeFloat => matches!(value, model::Value::Float(_)),
            model::ValueType::TypeDouble => matches!(value, model::Value::Double(_)),
            model::ValueType::TypeBoolArray => matches!(value, model::Value::BoolArray(_)),
            model::ValueType::TypeStringArray => matches!(value, model::Value::StringArray(_)),
            model::ValueType::TypeInt8Array => matches!(value, model::Value::Int8Array(_)),
            model::ValueType::TypeInt16Array => matches!(value, model::Value::Int16Array(_)),
            model::ValueType::TypeInt32Array => matches!(value, model::Value::Int32Array(_)),
            model::ValueType::TypeInt64Array => matches!(value, model::Value::Int64Array(_)),
            model::ValueType::TypeUint8Array => matches!(value, model::Value::Uint8Array(_)),
            model::ValueType::TypeUint16Array => matches!(value, model::Value::Uint16Array(_)),
            model::ValueType::TypeUint32Array => matches!(value, model::Value::Uint32Array(_)),
            model::ValueType::TypeUint64Array => matches!(value, model::Value::Uint64Array(_)),
            model::ValueType::TypeFloatArray => matches!(value, model::Value::FloatArray(_)),
            model::ValueType::TypeDoubleArray => matches!(value, model::Value::DoubleArray(_)),
            model::ValueType::TypeComplex(_) => matches!(value, model::Value::Complex(_)),
            model::ValueType::TypeComplexArray(_) => {
                matches!(value, model::Value::ComplexArray(_))
            }
        }
    }
}

impl model::SignalMetadata {
    pub fn is_match(&self, value: &model::SignalState) -> bool {
        self.data_type.is_match(&value.value)
    }
}

impl TryFrom<(&String, &Vec<model::VSSUserDefinedType>)> for model::ValueType {
    type Error = result::KuraError;

    fn try_from(tuple: (&String, &Vec<model::VSSUserDefinedType>)) -> Result<Self, Self::Error> {
        let (name, dict) = tuple;
        match name.to_lowercase().as_str() {
            "boolean" => Ok(model::ValueType::TypeBool),
            "string" => Ok(model::ValueType::TypeString),
            "int8" => Ok(model::ValueType::TypeInt8),
            "int16" => Ok(model::ValueType::TypeInt16),
            "int32" => Ok(model::ValueType::TypeInt32),
            "int64" => Ok(model::ValueType::TypeInt64),
            "uint8" => Ok(model::ValueType::TypeUint8),
            "uint16" => Ok(model::ValueType::TypeUint16),
            "uint32" => Ok(model::ValueType::TypeUint32),
            "uint64" => Ok(model::ValueType::TypeUint64),
            "float" => Ok(model::ValueType::TypeFloat),
            "double" => Ok(model::ValueType::TypeDouble),
            "boolean[]" => Ok(model::ValueType::TypeBoolArray),
            "string[]" => Ok(model::ValueType::TypeStringArray),
            "int8[]" => Ok(model::ValueType::TypeInt8Array),
            "int16[]" => Ok(model::ValueType::TypeInt16Array),
            "int32[]" => Ok(model::ValueType::TypeInt32Array),
            "int64[]" => Ok(model::ValueType::TypeInt64Array),
            "uint8[]" => Ok(model::ValueType::TypeUint8Array),
            "uint16[]" => Ok(model::ValueType::TypeUint16Array),
            "uint32[]" => Ok(model::ValueType::TypeUint32Array),
            "uint64[]" => Ok(model::ValueType::TypeUint64Array),
            "float[]" => Ok(model::ValueType::TypeFloatArray),
            "double[]" => Ok(model::ValueType::TypeDoubleArray),
            _ => {
                if let Some(base_name) = name.strip_suffix("[]") {
                    if dict
                        .iter()
                        .any(|user_defined_type| user_defined_type.name == base_name)
                    {
                        return Ok(model::ValueType::TypeComplexArray(base_name.to_string()));
                    }
                }
                if dict
                    .iter()
                    .any(|user_defined_type| user_defined_type.name == name.as_str())
                {
                    return Ok(model::ValueType::TypeComplex(name.to_string()));
                }
                result::KuraResult::Err(result::KuraError {
                    number: result::KuraErrorCode::BadRequest,
                    reason: result::KuraErrorReason::BadRequest,
                    description: Some(format!("{name} is not a valid datatype name")),
                })
            }
        }
    }
}

impl TryFrom<i32> for model::LeafType {
    type Error = result::KuraError;

    fn try_from(v: i32) -> Result<Self, Self::Error> {
        match v {
            x if x == model::LeafType::Branch as i32 => Ok(model::LeafType::Branch),
            x if x == model::LeafType::Sensor as i32 => Ok(model::LeafType::Sensor),
            x if x == model::LeafType::Attribute as i32 => Ok(model::LeafType::Attribute),
            x if x == model::LeafType::Actuator as i32 => Ok(model::LeafType::Actuator),
            _ => Err(result::KuraError {
                number: result::KuraErrorCode::BadRequest,
                reason: result::KuraErrorReason::InvalidData,
                description: Some(format!("{v} is invalied LeafType value.")),
            }),
        }
    }
}

impl FromStr for model::LeafType {
    type Err = Box<dyn std::error::Error>;
    fn from_str(s: &str) -> Result<Self, Box<dyn std::error::Error>> {
        match s.to_lowercase().as_str() {
            "branch" => Ok(model::LeafType::Branch),
            "sensor" => Ok(model::LeafType::Sensor),
            "attribute" => Ok(model::LeafType::Attribute),
            "actuator" => Ok(model::LeafType::Actuator),
            _ => Err(Box::new(io::Error::new(io::ErrorKind::InvalidData, s))),
        }
    }
}
