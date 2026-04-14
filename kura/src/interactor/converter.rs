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

use std::str::FromStr;

use crate::common::{FlexibleValue, MetadataNode};
use crate::interface::signal::{
    Config, DataPoint, KuraError, KuraErrorCode, KuraErrorReason, KuraResult, LeafType, Signal,
    Value, ValueType,
};

pub fn convert_metadata_to_signal(vss_path: &str, node: &MetadataNode) -> KuraResult<Signal> {
    let leaf_type = match node {
        MetadataNode::ActuatorMetadata { .. } => LeafType::Actuator,
        MetadataNode::AttributeMetadata { .. } => LeafType::Attribute,
        _ => LeafType::Sensor,
    };

    let data_type = match node {
        MetadataNode::ActuatorMetadata { datatype, .. } => datatype.clone(),
        MetadataNode::SensorMetadata { datatype, .. } => datatype.clone(),
        MetadataNode::AttributeMetadata { datatype, .. } => datatype.clone(),
        _ => "".to_string(),
    };
    let data_type = if let Ok(data_type) = ValueType::from_str(&data_type) {
        data_type
    } else {
        return KuraResult::Err(KuraError {
            number: KuraErrorCode::BadRequest,
            reason: KuraErrorReason::InvalidData,
            description: Some(format!("{data_type} is not a valid data type")),
        });
    };

    let deprecation = match node {
        MetadataNode::ActuatorMetadata { deprecation, .. } => deprecation.clone(),
        MetadataNode::SensorMetadata { deprecation, .. } => deprecation.clone(),
        MetadataNode::AttributeMetadata { deprecation, .. } => deprecation.clone(),
        _ => None,
    };

    let unit = match node {
        MetadataNode::ActuatorMetadata { unit, .. } => unit.clone(),
        MetadataNode::SensorMetadata { unit, .. } => unit.clone(),
        MetadataNode::AttributeMetadata { unit, .. } => unit.clone(),
        _ => None,
    };

    let min = match node {
        MetadataNode::ActuatorMetadata { min, .. } => Some(convert_flexible_value(min)),
        MetadataNode::SensorMetadata { min, .. } => Some(convert_flexible_value(min)),
        MetadataNode::AttributeMetadata { min, .. } => Some(convert_flexible_value(min)),
        _ => None,
    };

    let max = match node {
        MetadataNode::ActuatorMetadata { max, .. } => Some(convert_flexible_value(max)),
        MetadataNode::SensorMetadata { max, .. } => Some(convert_flexible_value(max)),
        MetadataNode::AttributeMetadata { max, .. } => Some(convert_flexible_value(max)),
        _ => None,
    };

    let array_size = match node {
        MetadataNode::ActuatorMetadata { arraysize, .. } => Some(*arraysize),
        MetadataNode::SensorMetadata { arraysize, .. } => Some(*arraysize),
        MetadataNode::AttributeMetadata { arraysize, .. } => Some(*arraysize),
        _ => None,
    };

    let description = match node {
        MetadataNode::ActuatorMetadata { description, .. } => description.clone(),
        MetadataNode::SensorMetadata { description, .. } => description.clone(),
        MetadataNode::AttributeMetadata { description, .. } => description.clone(),
        _ => None,
    };

    let comment = match node {
        MetadataNode::ActuatorMetadata { comment, .. } => comment.clone(),
        MetadataNode::SensorMetadata { comment, .. } => comment.clone(),
        MetadataNode::AttributeMetadata { comment, .. } => comment.clone(),
        _ => None,
    };

    let allowd: Option<Vec<String>> = match node {
        MetadataNode::ActuatorMetadata { allowed, .. } => Some(
            allowed
                .iter()
                .map(|x| convert_flexible_value(&Some(x.clone())).to_string())
                .collect(),
        ),
        MetadataNode::SensorMetadata { allowed, .. } => Some(
            allowed
                .iter()
                .map(|x| convert_flexible_value(&Some(x.clone())).to_string())
                .collect(),
        ),
        MetadataNode::AttributeMetadata { allowed, .. } => Some(
            allowed
                .iter()
                .map(|x| convert_flexible_value(&Some(x.clone())).to_string())
                .collect(),
        ),
        _ => None,
    };

    let default = match node {
        MetadataNode::ActuatorMetadata { default, .. } => convert_flexible_value(default),
        MetadataNode::SensorMetadata { default, .. } => convert_flexible_value(default),
        MetadataNode::AttributeMetadata { default, .. } => convert_flexible_value(default),
        _ => Value::Nan,
    };

    let current_datapoint = DataPoint {
        stored_timestamp: None,
        produced_timestamp: None,
        value: default.clone(),
        is_available_sensor: None,
        is_available_actuator: None,
    };
    let target_datapoint = DataPoint {
        stored_timestamp: None,
        produced_timestamp: None,
        value: Value::Nan,
        is_available_sensor: None,
        is_available_actuator: None,
    };

    let signal = Signal {
        path: vss_path.to_string(),
        current_datapoint,
        target_datapoint,
        config: Config {
            leaf_type,
            data_type,
            deprecation,
            unit,
            min,
            max,
            description,
            comment,
            allowd,
            default: Some(default),
            array_size,
            user_defined_items: None,
        },
    };

    Ok(signal)
}

fn convert_flexible_value(value: &Option<FlexibleValue>) -> Value {
    let value = match value.as_ref() {
        Some(value) => value,
        None => return Value::Nan,
    };

    match value {
        FlexibleValue::Int(value) => Value::Int64(*value),
        FlexibleValue::Float(value) => Value::Double(*value),
        FlexibleValue::Str(value) => Value::String(value.clone()),
        FlexibleValue::Bool(value) => Value::Bool(*value),
        FlexibleValue::Null => Value::Nan,
        FlexibleValue::Array(_) => convert_flexible_value_array(value),
        FlexibleValue::Object(_) => Value::Nan, // TODO: not supported yet
    }
}

fn convert_flexible_value_array(value: &FlexibleValue) -> Value {
    if let FlexibleValue::Array(values) = value {
        if values.is_empty() {
            return Value::Nan;
        }
        let first_value = &values[0];
        match first_value {
            FlexibleValue::Int(_) => {
                let arr: Vec<i32> = values
                    .iter()
                    .filter_map(|v| {
                        if let FlexibleValue::Int(i) = v {
                            Some(*i as i32)
                        } else {
                            None
                        }
                    })
                    .collect();
                Value::Int32Array(arr)
            }
            FlexibleValue::Float(_) => {
                let arr: Vec<f64> = values
                    .iter()
                    .filter_map(|v| {
                        if let FlexibleValue::Float(f) = v {
                            Some(*f)
                        } else {
                            None
                        }
                    })
                    .collect();
                Value::DoubleArray(arr)
            }
            FlexibleValue::Str(_) => {
                let arr: Vec<String> = values
                    .iter()
                    .filter_map(|v| {
                        if let FlexibleValue::Str(s) = v {
                            Some(s.clone())
                        } else {
                            None
                        }
                    })
                    .collect();
                Value::StringArray(arr)
            }
            FlexibleValue::Bool(_) => {
                let arr: Vec<bool> = values
                    .iter()
                    .filter_map(|v| {
                        if let FlexibleValue::Bool(b) = v {
                            Some(*b)
                        } else {
                            None
                        }
                    })
                    .collect();
                Value::BoolArray(arr)
            }
            FlexibleValue::Null => Value::Nan,
            FlexibleValue::Array(_) => Value::Nan,
            FlexibleValue::Object(_) => Value::Nan,
        }
    } else {
        Value::Nan
    }
}
