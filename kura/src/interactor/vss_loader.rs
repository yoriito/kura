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

use serde::Deserialize;
use serde::Serialize;
use std::collections::HashMap;

use crate::interface::{
    model,
    result::{self, MapExternalToKuraResult},
};

// ============================================================
// Public API
// ============================================================

pub fn load_user_defined_types(
    vss_path: &String,
    json_str: &str,
) -> result::KuraResult<Vec<model::VSSUserDefinedType>> {
    let json_value: serde_json::Value = serde_json::from_str(json_str).to_kura_result(
        result::KuraErrorCode::NotFound,
        result::KuraErrorReason::InvalidData,
        "JSON was not well-formatted".to_string(),
    )?;

    let complex_data_types = if let Some(complex_data_types) = json_value.get("ComplexDataTypes") {
        complex_data_types
    } else {
        return Ok([].to_vec());
    };

    let complex_types = if let Some(complex_types) = complex_data_types.get("Types") {
        complex_types
    } else {
        return Ok([].to_vec());
    };

    let complex_types_clildren = if let Some(complex_types_clildren) = complex_types.get("children")
    {
        complex_types_clildren
    } else {
        return Ok([].to_vec());
    };

    let types: HashMap<String, MetadataNode> =
        serde_json::from_str(&complex_types_clildren.to_string()).to_kura_result(
            result::KuraErrorCode::NotFound,
            result::KuraErrorReason::InvalidData,
            format!("failed to deserialize {vss_path}"),
        )?;

    let mut ret = vec![];
    for (struct_name, node) in types {
        let path = "Types.".to_owned() + &struct_name;
        ret.push(create_struct_dictionary(path, node)?);
    }

    Ok(ret)
}

pub fn load_metadata(
    vss_path: &String,
    json_str: &str,
    user_defined_datatypes: &Vec<model::VSSUserDefinedType>,
) -> result::KuraResult<HashMap<String, model::SignalMetadata>> {
    let json_value: serde_json::Value = serde_json::from_str(json_str).to_kura_result(
        result::KuraErrorCode::NotFound,
        result::KuraErrorReason::InvalidData,
        "JSON was not well-formatted".to_string(),
    )?;

    let wrapped_vehicle = if let Some(values) = json_value.get("Vehicle") {
        serde_json::json!({ "Vehicle": values })
    } else if let Some(values) = json_value.get("Server") {
        serde_json::json!({ "Server": values })
    } else {
        return Ok(HashMap::new());
    };

    let root: HashMap<String, MetadataNode> = serde_json::from_str(&wrapped_vehicle.to_string())
        .to_kura_result(
            result::KuraErrorCode::NotFound,
            result::KuraErrorReason::InvalidData,
            format!("failed to deserialize {vss_path}"),
        )?;

    let mut ret = HashMap::new();
    for (path, node) in root {
        ret.extend(create_metadata_recursive(
            path,
            node,
            user_defined_datatypes,
        )?);
    }
    Ok(ret)
}

// ============================================================
// Internal implementation
// ============================================================

#[derive(Clone, serde::Serialize, Debug, Deserialize)]
#[serde(untagged)]
pub enum FlexibleValue {
    Int(i64),
    Uint(u64),
    Float(f64),
    Str(String),
    Bool(bool),
    Null,
    Array(Vec<FlexibleValue>),
    Object(std::collections::HashMap<String, FlexibleValue>),
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct BranchMetadataInfo {
    pub description: Option<String>,
    #[serde(default)]
    pub children: HashMap<String, MetadataNode>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct AttributeMetadataInfo {
    pub datatype: String,
    pub description: Option<String>,
    #[serde(default)]
    pub deprecation: Option<String>,
    #[serde(default)]
    pub comment: Option<String>,
    #[serde(default)]
    pub min: Option<FlexibleValue>,
    #[serde(default)]
    pub max: Option<FlexibleValue>,
    #[serde(default)]
    pub unit: Option<String>,
    #[serde(default)]
    pub pattern: Option<String>,
    #[serde(default)]
    pub default: Option<FlexibleValue>,
    #[serde(default)]
    pub allowed: Vec<FlexibleValue>,
    #[serde(default)]
    pub arraysize: u64,
    #[serde(default)]
    pub user_defined_items: HashMap<String, String>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct SensorMetadataInfo {
    pub datatype: String,
    pub description: Option<String>,
    #[serde(default)]
    pub deprecation: Option<String>,
    #[serde(default)]
    pub comment: Option<String>,
    #[serde(default)]
    pub min: Option<FlexibleValue>,
    #[serde(default)]
    pub max: Option<FlexibleValue>,
    #[serde(default)]
    pub unit: Option<String>,
    #[serde(default)]
    pub pattern: Option<String>,
    #[serde(default)]
    pub default: Option<FlexibleValue>,
    #[serde(default)]
    pub allowed: Vec<FlexibleValue>,
    #[serde(default)]
    pub arraysize: u64,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct ActuatorMetadataInfo {
    pub datatype: String,
    pub description: Option<String>,
    #[serde(default)]
    pub deprecation: Option<String>,
    #[serde(default)]
    pub comment: Option<String>,
    #[serde(default)]
    pub min: Option<FlexibleValue>,
    #[serde(default)]
    pub max: Option<FlexibleValue>,
    #[serde(default)]
    pub unit: Option<String>,
    #[serde(default)]
    pub pattern: Option<String>,
    #[serde(default)]
    pub default: Option<FlexibleValue>,
    #[serde(default)]
    pub allowed: Vec<FlexibleValue>,
    #[serde(default)]
    pub arraysize: u64,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct StructMetadataInfo {
    pub description: Option<String>,
    #[serde(default)]
    pub children: HashMap<String, MetadataNode>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct PropertyMetadataInfo {
    pub datatype: String,
    pub description: Option<String>,
    #[serde(default)]
    pub deprecation: Option<String>,
    #[serde(default)]
    pub comment: Option<String>,
    #[serde(default)]
    pub min: Option<FlexibleValue>,
    #[serde(default)]
    pub max: Option<FlexibleValue>,
    #[serde(default)]
    pub unit: Option<String>,
    #[serde(default)]
    pub pattern: Option<String>,
    #[serde(default)]
    pub default: Option<FlexibleValue>,
    #[serde(default)]
    pub allowed: Vec<FlexibleValue>,
    #[serde(default)]
    pub arraysize: u64,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
#[serde(tag = "type")]
pub enum MetadataNode {
    #[serde(rename = "branch")]
    Branch(BranchMetadataInfo),

    #[serde(rename = "attribute")]
    AttributeMetadata(AttributeMetadataInfo),

    #[serde(rename = "sensor")]
    SensorMetadata(SensorMetadataInfo),

    #[serde(rename = "actuator")]
    ActuatorMetadata(ActuatorMetadataInfo),

    #[serde(rename = "struct")]
    StructMetadata(StructMetadataInfo),

    #[serde(rename = "property")]
    PropertyMetadata(PropertyMetadataInfo),
}

fn create_struct_dictionary(
    path: String,
    node: MetadataNode,
) -> result::KuraResult<model::VSSUserDefinedType> {
    if let MetadataNode::StructMetadata(StructMetadataInfo {
        description,
        children,
    }) = node
    {
        let mut cur_struct = model::VSSUserDefinedType {
            name: path.clone(),
            children: HashMap::new(),
            description,
            deprecation: None,
            comment: None,
        };
        for (child_path, child_node) in children {
            if let MetadataNode::PropertyMetadata(property_metadata) = child_node {
                let property_path = format!("{path}.{child_path}");
                let property = create_property_metadata(property_metadata)?;
                cur_struct.children.insert(property_path, property);
            }
        }
        return Ok(cur_struct);
    }
    result::KuraResult::Err(result::KuraError {
        number: result::KuraErrorCode::BadRequest,
        reason: result::KuraErrorReason::InvalidData,
        description: Some(format!("{path} is not a struct node")),
    })
}

fn create_metadata_recursive(
    path: String,
    node: MetadataNode,
    user_defined_datatypes: &Vec<model::VSSUserDefinedType>,
) -> result::KuraResult<HashMap<String, model::SignalMetadata>> {
    if let MetadataNode::Branch(BranchMetadataInfo {
        description: _,
        children,
    }) = node
    {
        let mut ret = HashMap::new();
        for (child_path, child_node) in children {
            ret.extend(create_metadata_recursive(
                path.clone() + "." + &child_path,
                child_node,
                user_defined_datatypes,
            )?);
        }
        return Ok(ret);
    }
    Ok(HashMap::from([(
        path.clone(),
        create_single_metadata(node, user_defined_datatypes)?,
    )]))
}

fn create_property_metadata(
    loaded_metadata: PropertyMetadataInfo,
) -> result::KuraResult<model::PropertyMetadata> {
    let leaf_type = model::LeafType::Property;
    let data_type = loaded_metadata.datatype;
    let data_type = if let Ok(data_type) = model::ValueType::try_from((&data_type, &[].to_vec())) {
        data_type
    } else {
        // TODO:
        // Support resolving symbols declared later via multi-pass compilation.
        model::ValueType::TypeComplex(data_type)
    };

    let deprecation = loaded_metadata.deprecation;
    let unit = loaded_metadata.unit;
    let min = Some(convert_flexible_value_typed(
        &loaded_metadata.min,
        &data_type,
    )?);
    let max = Some(convert_flexible_value_typed(
        &loaded_metadata.max,
        &data_type,
    )?);
    let array_size = Some(loaded_metadata.arraysize);
    let description = loaded_metadata.description;
    let comment = loaded_metadata.comment;
    let allowd: Option<Vec<String>> = Some(
        loaded_metadata
            .allowed
            .iter()
            .map(|x| convert_flexible_value(&Some(x.clone())).to_string())
            .collect(),
    );
    let default = convert_flexible_value_typed(&loaded_metadata.default, &data_type)?;
    let metadata = model::PropertyMetadata {
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
        pattern: None, // TODO
    };
    Ok(metadata)
}

fn create_single_metadata(
    node: MetadataNode,
    user_defined_datatypes: &Vec<model::VSSUserDefinedType>,
) -> result::KuraResult<model::SignalMetadata> {
    match node {
        MetadataNode::ActuatorMetadata(loaded_metadata) => {
            create_actuator_metadata(loaded_metadata, user_defined_datatypes)
        }
        MetadataNode::AttributeMetadata(loaded_metadata) => {
            create_attribute_metadata(loaded_metadata, user_defined_datatypes)
        }
        MetadataNode::Branch(loaded_metadata) => {
            create_branch_metadata(loaded_metadata, user_defined_datatypes)
        }
        MetadataNode::SensorMetadata(loaded_metadata) => {
            create_sensor_metadata(loaded_metadata, user_defined_datatypes)
        }
        MetadataNode::StructMetadata(loaded_metadata) => {
            create_struct_metadata(loaded_metadata, user_defined_datatypes)
        }
        _ => unreachable!(),
    }
}

fn create_branch_metadata(
    _: BranchMetadataInfo,
    _: &Vec<model::VSSUserDefinedType>,
) -> result::KuraResult<model::SignalMetadata> {
    unreachable!()
}

fn create_attribute_metadata(
    loaded_metadata: AttributeMetadataInfo,
    dict: &Vec<model::VSSUserDefinedType>,
) -> result::KuraResult<model::SignalMetadata> {
    let leaf_type = model::LeafType::Attribute;
    let data_type = loaded_metadata.datatype;
    let data_type = if let Ok(data_type) = model::ValueType::try_from((&data_type, dict)) {
        data_type
    } else {
        return result::KuraResult::Err(result::KuraError {
            number: result::KuraErrorCode::BadRequest,
            reason: result::KuraErrorReason::InvalidData,
            description: Some(format!("{data_type} is not a valid data type")),
        });
    };
    let deprecation = loaded_metadata.deprecation;
    let unit = loaded_metadata.unit;
    let min = Some(convert_flexible_value_typed(
        &loaded_metadata.min,
        &data_type,
    )?);
    let max = Some(convert_flexible_value_typed(
        &loaded_metadata.max,
        &data_type,
    )?);
    let array_size = Some(loaded_metadata.arraysize);
    let description = loaded_metadata.description;
    let comment = loaded_metadata.comment;
    let allowd: Option<Vec<String>> = Some(
        loaded_metadata
            .allowed
            .iter()
            .map(|x| convert_flexible_value(&Some(x.clone())).to_string())
            .collect(),
    );
    let default = convert_flexible_value_typed(&loaded_metadata.default, &data_type)?;
    let metadata = model::SignalMetadata {
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
        pattern: None,
    };
    Ok(metadata)
}

fn create_actuator_metadata(
    loaded_metadata: ActuatorMetadataInfo,
    dict: &Vec<model::VSSUserDefinedType>,
) -> result::KuraResult<model::SignalMetadata> {
    let leaf_type = model::LeafType::Actuator;
    let data_type = loaded_metadata.datatype;
    let data_type = if let Ok(data_type) = model::ValueType::try_from((&data_type, dict)) {
        data_type
    } else {
        return result::KuraResult::Err(result::KuraError {
            number: result::KuraErrorCode::BadRequest,
            reason: result::KuraErrorReason::InvalidData,
            description: Some(format!("{data_type} is not a valid data type")),
        });
    };
    let deprecation = loaded_metadata.deprecation;
    let unit = loaded_metadata.unit;
    let min = Some(convert_flexible_value_typed(
        &loaded_metadata.min,
        &data_type,
    )?);
    let max = Some(convert_flexible_value_typed(
        &loaded_metadata.max,
        &data_type,
    )?);
    let array_size = Some(loaded_metadata.arraysize);
    let description = loaded_metadata.description;
    let comment = loaded_metadata.comment;
    let allowd: Option<Vec<String>> = Some(
        loaded_metadata
            .allowed
            .iter()
            .map(|x| convert_flexible_value(&Some(x.clone())).to_string())
            .collect(),
    );
    let default = convert_flexible_value_typed(&loaded_metadata.default, &data_type)?;
    let metadata = model::SignalMetadata {
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
        pattern: None,
    };
    Ok(metadata)
}

fn create_sensor_metadata(
    loaded_metadata: SensorMetadataInfo,
    dict: &Vec<model::VSSUserDefinedType>,
) -> result::KuraResult<model::SignalMetadata> {
    let leaf_type = model::LeafType::Sensor;
    let data_type = loaded_metadata.datatype;
    let data_type = if let Ok(data_type) = model::ValueType::try_from((&data_type, dict)) {
        data_type
    } else {
        return result::KuraResult::Err(result::KuraError {
            number: result::KuraErrorCode::BadRequest,
            reason: result::KuraErrorReason::InvalidData,
            description: Some(format!("{data_type} is not a valid data type")),
        });
    };
    let deprecation = loaded_metadata.deprecation;
    let unit = loaded_metadata.unit;
    let min = Some(convert_flexible_value_typed(
        &loaded_metadata.min,
        &data_type,
    )?);
    let max = Some(convert_flexible_value_typed(
        &loaded_metadata.max,
        &data_type,
    )?);
    let array_size = Some(loaded_metadata.arraysize);
    let description = loaded_metadata.description;
    let comment = loaded_metadata.comment;
    let allowd: Option<Vec<String>> = Some(
        loaded_metadata
            .allowed
            .iter()
            .map(|x| convert_flexible_value(&Some(x.clone())).to_string())
            .collect(),
    );
    let default = convert_flexible_value_typed(&loaded_metadata.default, &data_type)?;
    let metadata = model::SignalMetadata {
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
        pattern: None,
    };
    Ok(metadata)
}

fn create_struct_metadata(
    _: StructMetadataInfo,
    _: &Vec<model::VSSUserDefinedType>,
) -> result::KuraResult<model::SignalMetadata> {
    unreachable!()
}

fn invalid_metadata_value(
    data_type: &model::ValueType,
    value: &FlexibleValue,
) -> result::KuraError {
    result::KuraError {
        number: result::KuraErrorCode::BadRequest,
        reason: result::KuraErrorReason::InvalidData,
        description: Some(format!(
            "value {value:?} does not match metadata datatype {data_type:?}"
        )),
    }
}

fn convert_number_value<T>(
    value: &FlexibleValue,
    data_type: &model::ValueType,
    map: impl FnOnce(T) -> model::Value,
) -> result::KuraResult<model::Value>
where
    T: TryFrom<i64> + TryFrom<u64>,
{
    convert_number_element(value, data_type).map(map)
}

fn convert_flexible_value_typed(
    value: &Option<FlexibleValue>,
    data_type: &model::ValueType,
) -> result::KuraResult<model::Value> {
    let value = match value.as_ref() {
        Some(value) => value,
        None => return Ok(model::Value::Nan),
    };

    match data_type {
        model::ValueType::TypeNAN => Ok(model::Value::Nan),
        model::ValueType::TypeBool => {
            convert_bool_element(value, data_type).map(model::Value::Bool)
        }
        model::ValueType::TypeString => {
            convert_string_element(value, data_type).map(model::Value::String)
        }
        model::ValueType::TypeInt8 => convert_number_value(value, data_type, model::Value::Int8),
        model::ValueType::TypeInt16 => convert_number_value(value, data_type, model::Value::Int16),
        model::ValueType::TypeInt32 => convert_number_value(value, data_type, model::Value::Int32),
        model::ValueType::TypeInt64 => convert_number_value(value, data_type, model::Value::Int64),
        model::ValueType::TypeUint8 => convert_number_value(value, data_type, model::Value::Uint8),
        model::ValueType::TypeUint16 => {
            convert_number_value(value, data_type, model::Value::Uint16)
        }
        model::ValueType::TypeUint32 => {
            convert_number_value(value, data_type, model::Value::Uint32)
        }
        model::ValueType::TypeUint64 => {
            convert_number_value(value, data_type, model::Value::Uint64)
        }
        model::ValueType::TypeFloat => {
            convert_float_element(value, data_type).map(model::Value::Float)
        }
        model::ValueType::TypeDouble => {
            convert_double_element(value, data_type).map(model::Value::Double)
        }
        model::ValueType::TypeBoolArray => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeStringArray => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeInt8Array => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeInt16Array => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeInt32Array => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeInt64Array => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeUint8Array => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeUint16Array => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeUint32Array => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeUint64Array => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeFloatArray => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeDoubleArray => convert_flexible_value_array_typed(value, data_type),
        model::ValueType::TypeComplex(_) => match value {
            FlexibleValue::Object(obj) => {
                let mut complex_map = HashMap::new();
                for (key, val) in obj {
                    complex_map.insert(key.clone(), convert_flexible_value(&Some(val.clone())));
                }
                Ok(model::Value::Complex(complex_map))
            }
            FlexibleValue::Null => Ok(model::Value::Nan),
            _ => Err(invalid_metadata_value(data_type, value)),
        },
        model::ValueType::TypeComplexArray(_) => {
            convert_flexible_value_array_typed(value, data_type)
        }
    }
}

fn convert_bool_element(
    value: &FlexibleValue,
    data_type: &model::ValueType,
) -> result::KuraResult<bool> {
    match value {
        FlexibleValue::Bool(bool_value) => Ok(*bool_value),
        _ => Err(invalid_metadata_value(data_type, value)),
    }
}

fn convert_string_element(
    value: &FlexibleValue,
    data_type: &model::ValueType,
) -> result::KuraResult<String> {
    match value {
        FlexibleValue::Str(str_value) => Ok(str_value.clone()),
        _ => Err(invalid_metadata_value(data_type, value)),
    }
}

fn convert_number_element<T>(
    value: &FlexibleValue,
    data_type: &model::ValueType,
) -> result::KuraResult<T>
where
    T: TryFrom<i64> + TryFrom<u64>,
{
    match value {
        FlexibleValue::Int(int_value) => {
            T::try_from(*int_value).map_err(|_| invalid_metadata_value(data_type, value))
        }
        FlexibleValue::Uint(uint_value) => {
            T::try_from(*uint_value).map_err(|_| invalid_metadata_value(data_type, value))
        }
        _ => Err(invalid_metadata_value(data_type, value)),
    }
}

fn convert_float_element(
    value: &FlexibleValue,
    data_type: &model::ValueType,
) -> result::KuraResult<f32> {
    match value {
        FlexibleValue::Float(float_value) => Ok(*float_value as f32),
        FlexibleValue::Int(int_value) => Ok(*int_value as f32),
        FlexibleValue::Uint(uint_value) => Ok(*uint_value as f32),
        _ => Err(invalid_metadata_value(data_type, value)),
    }
}

fn convert_double_element(
    value: &FlexibleValue,
    data_type: &model::ValueType,
) -> result::KuraResult<f64> {
    match value {
        FlexibleValue::Float(float_value) => Ok(*float_value),
        FlexibleValue::Int(int_value) => Ok(*int_value as f64),
        FlexibleValue::Uint(uint_value) => Ok(*uint_value as f64),
        _ => Err(invalid_metadata_value(data_type, value)),
    }
}

fn collect_array_elements<T>(
    values: &[FlexibleValue],
    data_type: &model::ValueType,
    convert: fn(&FlexibleValue, &model::ValueType) -> result::KuraResult<T>,
    map: impl FnOnce(Vec<T>) -> model::Value,
) -> result::KuraResult<model::Value> {
    let elements = values
        .iter()
        .map(|value| convert(value, data_type))
        .collect::<result::KuraResult<Vec<T>>>()?;
    Ok(map(elements))
}

fn convert_flexible_value_array_typed(
    value: &FlexibleValue,
    data_type: &model::ValueType,
) -> result::KuraResult<model::Value> {
    let values = match value {
        FlexibleValue::Array(values) => values,
        _ => return Err(invalid_metadata_value(data_type, value)),
    };

    if values.is_empty() {
        return Ok(model::Value::Nan);
    }

    match data_type {
        model::ValueType::TypeBoolArray => collect_array_elements(
            values,
            data_type,
            convert_bool_element,
            model::Value::BoolArray,
        ),
        model::ValueType::TypeStringArray => collect_array_elements(
            values,
            data_type,
            convert_string_element,
            model::Value::StringArray,
        ),
        model::ValueType::TypeInt8Array => collect_array_elements(
            values,
            data_type,
            convert_number_element,
            model::Value::Int8Array,
        ),
        model::ValueType::TypeInt16Array => collect_array_elements(
            values,
            data_type,
            convert_number_element,
            model::Value::Int16Array,
        ),
        model::ValueType::TypeInt32Array => collect_array_elements(
            values,
            data_type,
            convert_number_element,
            model::Value::Int32Array,
        ),
        model::ValueType::TypeInt64Array => collect_array_elements(
            values,
            data_type,
            convert_number_element,
            model::Value::Int64Array,
        ),
        model::ValueType::TypeUint8Array => collect_array_elements(
            values,
            data_type,
            convert_number_element,
            model::Value::Uint8Array,
        ),
        model::ValueType::TypeUint16Array => collect_array_elements(
            values,
            data_type,
            convert_number_element,
            model::Value::Uint16Array,
        ),
        model::ValueType::TypeUint32Array => collect_array_elements(
            values,
            data_type,
            convert_number_element,
            model::Value::Uint32Array,
        ),
        model::ValueType::TypeUint64Array => collect_array_elements(
            values,
            data_type,
            convert_number_element,
            model::Value::Uint64Array,
        ),
        model::ValueType::TypeFloatArray => collect_array_elements(
            values,
            data_type,
            convert_float_element,
            model::Value::FloatArray,
        ),
        model::ValueType::TypeDoubleArray => collect_array_elements(
            values,
            data_type,
            convert_double_element,
            model::Value::DoubleArray,
        ),
        _ => Err(invalid_metadata_value(data_type, value)),
    }
}

fn convert_flexible_value(value: &Option<FlexibleValue>) -> model::Value {
    let value = match value.as_ref() {
        Some(value) => value,
        None => return model::Value::Nan,
    };

    match value {
        FlexibleValue::Int(value) => model::Value::Int64(*value),
        FlexibleValue::Uint(value) => model::Value::Uint64(*value),
        FlexibleValue::Float(value) => model::Value::Double(*value),
        FlexibleValue::Str(value) => model::Value::String(value.clone()),
        FlexibleValue::Bool(value) => model::Value::Bool(*value),
        FlexibleValue::Null => model::Value::Nan,
        FlexibleValue::Array(_) => convert_flexible_value_array(value),
        FlexibleValue::Object(obj) => {
            let mut complex_map = HashMap::new();
            for (key, val) in obj {
                complex_map.insert(key.clone(), convert_flexible_value(&Some(val.clone())));
            }
            model::Value::Complex(complex_map)
        }
    }
}

fn convert_flexible_value_array(value: &FlexibleValue) -> model::Value {
    if let FlexibleValue::Array(values) = value {
        if values.is_empty() {
            return model::Value::Nan;
        }
        let first_value = &values[0];
        match first_value {
            FlexibleValue::Int(_) | FlexibleValue::Uint(_) => {
                let arr: Vec<i32> = values
                    .iter()
                    .filter_map(|v| match v {
                        FlexibleValue::Int(i) => Some(*i as i32),
                        FlexibleValue::Uint(i) => Some(*i as i32),
                        _ => None,
                    })
                    .collect();
                model::Value::Int32Array(arr)
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
                model::Value::DoubleArray(arr)
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
                model::Value::StringArray(arr)
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
                model::Value::BoolArray(arr)
            }
            FlexibleValue::Null => model::Value::Nan,
            FlexibleValue::Array(_) => model::Value::Nan,
            FlexibleValue::Object(_) => {
                let arr: Vec<HashMap<String, model::Value>> = values
                    .iter()
                    .filter_map(|v| {
                        if let FlexibleValue::Object(obj) = v {
                            let mut complex_map = HashMap::new();
                            for (key, val) in obj {
                                complex_map.insert(
                                    key.clone(),
                                    convert_flexible_value(&Some(val.clone())),
                                );
                            }
                            Some(complex_map)
                        } else {
                            None
                        }
                    })
                    .collect();
                model::Value::ComplexArray(arr)
            }
        }
    } else {
        model::Value::Nan
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn typed_metadata_values_follow_int8_datatype() {
        let min = convert_flexible_value_typed(
            &Some(FlexibleValue::Int(-128)),
            &model::ValueType::TypeInt8,
        )
        .unwrap_or_else(|error| panic!("int8 min should convert: {error}"));
        let max = convert_flexible_value_typed(
            &Some(FlexibleValue::Int(127)),
            &model::ValueType::TypeInt8,
        )
        .unwrap_or_else(|error| panic!("int8 max should convert: {error}"));

        assert_eq!(min, model::Value::Int8(-128));
        assert_eq!(max, model::Value::Int8(127));
    }

    #[test]
    fn typed_metadata_values_reject_out_of_range_int8() {
        let result = convert_flexible_value_typed(
            &Some(FlexibleValue::Int(128)),
            &model::ValueType::TypeInt8,
        );

        assert!(result.is_err());
    }
}
