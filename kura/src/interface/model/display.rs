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

use std::fmt::Display;

impl Display for model::SignalState {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{{ \"value\": \"{}\", }}", self.value,)
    }
}

impl Display for model::SignalMetadata {
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

impl Display for model::VSSUserDefinedType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let entries = self
            .children
            .iter()
            .map(|(key, value)| format!("\"{key}\": {value}"))
            .collect::<Vec<_>>()
            .join(", ");

        write!(f, "{{ \"properties\": {{ {entries} }} }}",)
    }
}

impl Display for model::PropertyMetadata {
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

impl Display for model::Value {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            model::Value::Nan => write!(f, "None"),
            model::Value::Bool(v) => write!(f, "{v}"),
            model::Value::String(v) => write!(f, "{v}"),
            model::Value::Int8(v) => write!(f, "{v}"),
            model::Value::Int16(v) => write!(f, "{v}"),
            model::Value::Int32(v) => write!(f, "{v}"),
            model::Value::Int64(v) => write!(f, "{v}"),
            model::Value::Uint8(v) => write!(f, "{v}"),
            model::Value::Uint16(v) => write!(f, "{v}"),
            model::Value::Uint32(v) => write!(f, "{v}"),
            model::Value::Uint64(v) => write!(f, "{v}"),
            model::Value::Float(v) => write!(f, "{v}"),
            model::Value::Double(v) => write!(f, "{v}"),
            model::Value::BoolArray(v) => write!(f, "{v:?}"),
            model::Value::StringArray(v) => write!(f, "{v:?}"),
            model::Value::Int8Array(v) => write!(f, "{v:?}"),
            model::Value::Int16Array(v) => write!(f, "{v:?}"),
            model::Value::Int32Array(v) => write!(f, "{v:?}"),
            model::Value::Int64Array(v) => write!(f, "{v:?}"),
            model::Value::Uint8Array(v) => write!(f, "{v:?}"),
            model::Value::Uint16Array(v) => write!(f, "{v:?}"),
            model::Value::Uint32Array(v) => write!(f, "{v:?}"),
            model::Value::Uint64Array(v) => write!(f, "{v:?}"),
            model::Value::FloatArray(v) => write!(f, "{v:?}"),
            model::Value::DoubleArray(v) => write!(f, "{v:?}"),
            model::Value::Complex(v) => write!(f, "{v:?}"),
            model::Value::ComplexArray(v) => write!(f, "{v:?}"),
        }
    }
}

impl Display for model::LeafType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let name = match self {
            model::LeafType::Branch => String::from("Branch"),
            model::LeafType::Sensor => String::from("Sensor"),
            model::LeafType::Attribute => String::from("Attribute"),
            model::LeafType::Actuator => String::from("Actuator"),
            model::LeafType::Property => String::from("Property"),
        };
        write!(f, "{name}")
    }
}

impl Display for model::ValueType {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let data_type_str = match self {
            model::ValueType::TypeNAN => "NAN".to_string(),
            model::ValueType::TypeBool => "boolean".to_string(),
            model::ValueType::TypeString => "string".to_string(),
            model::ValueType::TypeInt8 => "int8".to_string(),
            model::ValueType::TypeInt16 => "int16".to_string(),
            model::ValueType::TypeInt32 => "int32".to_string(),
            model::ValueType::TypeInt64 => "int64".to_string(),
            model::ValueType::TypeUint8 => "uint8".to_string(),
            model::ValueType::TypeUint16 => "uint16".to_string(),
            model::ValueType::TypeUint32 => "uint32".to_string(),
            model::ValueType::TypeUint64 => "uint64".to_string(),
            model::ValueType::TypeFloat => "float".to_string(),
            model::ValueType::TypeDouble => "double".to_string(),
            model::ValueType::TypeBoolArray => "boolean[]".to_string(),
            model::ValueType::TypeStringArray => "string[]".to_string(),
            model::ValueType::TypeInt8Array => "int8[]".to_string(),
            model::ValueType::TypeInt16Array => "int16[]".to_string(),
            model::ValueType::TypeInt32Array => "int32[]".to_string(),
            model::ValueType::TypeInt64Array => "int64[]".to_string(),
            model::ValueType::TypeUint8Array => "uint8[]".to_string(),
            model::ValueType::TypeUint16Array => "uint16[]".to_string(),
            model::ValueType::TypeUint32Array => "uint32[]".to_string(),
            model::ValueType::TypeUint64Array => "uint64[]".to_string(),
            model::ValueType::TypeFloatArray => "float[]".to_string(),
            model::ValueType::TypeDoubleArray => "double[]".to_string(),
            model::ValueType::TypeComplex(user_defined_type) => user_defined_type.clone(),
            model::ValueType::TypeComplexArray(user_defined_type) => {
                format!("{user_defined_type}[]")
            }
        };
        write!(f, "{data_type_str}")
    }
}
