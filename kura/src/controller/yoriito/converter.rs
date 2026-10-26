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

use crate::controller::yoriito::common::viss as common_viss;
use crate::interface::{model, result, subscription};
use std::collections::HashMap;
use std::convert::TryFrom;
use tracing::error;

// Result Converters
pub trait VissErrorExt<T> {
    fn to_viss_error(self) -> Result<T, Box<tonic::Status>>;
}

impl<T> VissErrorExt<T> for result::KuraResult<T> {
    fn to_viss_error(self) -> Result<T, Box<tonic::Status>> {
        match self {
            Ok(val) => Ok(val),
            Err(e) => Err(Box::new(tonic::Status::internal(e.to_string()))),
        }
    }
}

pub trait OptionStatusExt<T> {
    #[allow(dead_code)]
    fn or_not_found(self, msg: &str) -> Result<T, Box<tonic::Status>>;
}

impl<T> OptionStatusExt<T> for Option<T> {
    fn or_not_found(self, msg: &str) -> Result<T, Box<tonic::Status>> {
        error!("{}", msg);
        self.ok_or_else(|| Box::new(tonic::Status::not_found(msg)))
    }
}

// Filter Converters
impl TryFrom<&common_viss::InspectFilter> for subscription::PathsFilter {
    type Error = anyhow::Error;
    fn try_from(v: &common_viss::InspectFilter) -> Result<Self, Self::Error> {
        match &v.parameter_variant {
            None => Err(std::io::Error::new(
                std::io::ErrorKind::NotFound,
                "paths filter not found".to_string(),
            )
            .into()),
            Some(common_viss::inspect_filter::ParameterVariant::PathsParameter(paths_filter)) => {
                Ok(subscription::PathsFilter {
                    paths: paths_filter.paths.clone(),
                })
            }
        }
    }
}

impl TryFrom<&common_viss::SubscribeFilter> for subscription::PathsFilter {
    type Error = anyhow::Error;
    fn try_from(v: &common_viss::SubscribeFilter) -> Result<Self, Self::Error> {
        match &v.parameter_variant {
            Some(common_viss::subscribe_filter::ParameterVariant::PathsParameter(paths_filter)) => {
                Ok(subscription::PathsFilter {
                    paths: paths_filter.paths.clone(),
                })
            }
            _ => Err(std::io::Error::new(
                std::io::ErrorKind::NotFound,
                "paths filter not found".to_string(),
            )
            .into()),
        }
    }
}

pub trait FilterUtil {
    fn to_paths_filter(&self) -> Option<subscription::PathsFilter>;
    fn to_misc_filter(&self) -> Option<subscription::MiscFilter>;
}

impl FilterUtil for common_viss::SubscribeFilter {
    fn to_paths_filter(&self) -> Option<subscription::PathsFilter> {
        match &self.parameter_variant.clone()? {
            common_viss::subscribe_filter::ParameterVariant::PathsParameter(v) => {
                Some(subscription::PathsFilter {
                    paths: v.paths.clone(),
                })
            }
            common_viss::subscribe_filter::ParameterVariant::ComboFilter(v) => {
                Some(subscription::PathsFilter {
                    paths: v.paths_parameter.clone()?.paths,
                })
            }
            _ => None,
        }
    }

    fn to_misc_filter(&self) -> Option<subscription::MiscFilter> {
        match &self.parameter_variant.clone()? {
            common_viss::subscribe_filter::ParameterVariant::MiscParameter(v) => Some(
                subscription::MiscFilter::from(&v.parameter_variant.clone()?),
            ),
            common_viss::subscribe_filter::ParameterVariant::ComboFilter(v) => Some(
                subscription::MiscFilter::from(&v.misc_parameter.clone()?.parameter_variant?),
            ),
            _ => None,
        }
    }
}

impl FilterUtil for common_viss::GetFilter {
    fn to_paths_filter(&self) -> Option<subscription::PathsFilter> {
        match self.parameter_variant.clone()? {
            common_viss::get_filter::ParameterVariant::PathsParameter(v) => {
                Some(subscription::PathsFilter {
                    paths: v.paths.clone(),
                })
            }
            common_viss::get_filter::ParameterVariant::HistoryParameter(_) => None,
        }
    }

    fn to_misc_filter(&self) -> Option<subscription::MiscFilter> {
        match self.parameter_variant.clone()? {
            common_viss::get_filter::ParameterVariant::PathsParameter(_) => None,
            common_viss::get_filter::ParameterVariant::HistoryParameter(v) => {
                Some(subscription::MiscFilter::from(&v))
            }
        }
    }
}

impl FilterUtil for common_viss::InspectFilter {
    fn to_paths_filter(&self) -> Option<subscription::PathsFilter> {
        match self.parameter_variant.clone()? {
            common_viss::inspect_filter::ParameterVariant::PathsParameter(v) => {
                Some(subscription::PathsFilter {
                    paths: v.paths.clone(),
                })
            }
        }
    }

    fn to_misc_filter(&self) -> Option<subscription::MiscFilter> {
        None
    }
}

impl From<&common_viss::PathsFilter> for subscription::PathsFilter {
    fn from(v: &common_viss::PathsFilter) -> Self {
        subscription::PathsFilter {
            paths: v.paths.clone(),
        }
    }
}

impl From<&common_viss::TimebasedFilter> for subscription::MiscFilter {
    fn from(v: &common_viss::TimebasedFilter) -> Self {
        subscription::MiscFilter::Timebased(subscription::TimebasedFilter { interval: v.period })
    }
}

impl From<&common_viss::ChangeFilter> for subscription::MiscFilter {
    fn from(v: &common_viss::ChangeFilter) -> Self {
        let operator = match common_viss::Operator::try_from(v.logic_op) {
            Ok(v) => v,
            Err(_) => return subscription::MiscFilter::Invalid,
        };
        subscription::MiscFilter::Change(subscription::ChangeFilter {
            opcode: (&operator).into(),
            offset: v.diff,
        })
    }
}

impl From<&common_viss::RangeFilter> for subscription::MiscFilter {
    fn from(v: &common_viss::RangeFilter) -> Self {
        let mut conditions: Vec<subscription::RangeCondition> = [].to_vec();
        let mut combination = None;
        match &v.range_type {
            Some(common_viss::range_filter::RangeType::Single(v)) => {
                let operator = match common_viss::Operator::try_from(v.boundary_op) {
                    Ok(v) => v,
                    Err(_) => return subscription::MiscFilter::Invalid,
                };
                conditions.push(subscription::RangeCondition {
                    opcode: (&operator).into(),
                    boundary: v.boundary,
                });
            }
            Some(common_viss::range_filter::RangeType::Multiple(v)) => {
                for cur_condition in &v.conditions {
                    if let Some(condition) = cur_condition.combination_op {
                        let condition = match common_viss::Condition::try_from(condition) {
                            Ok(v) => v,
                            Err(_) => return subscription::MiscFilter::Invalid,
                        };
                        combination = Some((&condition).into());
                    }
                    let operator = match common_viss::Operator::try_from(cur_condition.boundary_op)
                    {
                        Ok(v) => v,
                        Err(_) => return subscription::MiscFilter::Invalid,
                    };
                    conditions.push(subscription::RangeCondition {
                        opcode: (&operator).into(),
                        boundary: cur_condition.boundary,
                    });
                }
            }
            None => {}
        }
        subscription::MiscFilter::Range(subscription::RangeFilter {
            conditions,
            combination,
        })
    }
}

impl From<&common_viss::CurvelogFilter> for subscription::MiscFilter {
    fn from(v: &common_viss::CurvelogFilter) -> Self {
        subscription::MiscFilter::Curvelog(subscription::CurvelogFilter {
            maxerr: v.maxerr,
            bufsize: v.bufsize,
        })
    }
}

impl From<&common_viss::HistoryFilter> for subscription::MiscFilter {
    fn from(v: &common_viss::HistoryFilter) -> Self {
        subscription::MiscFilter::History(subscription::HistoryFilter {
            time_period: v.time_period.clone(),
        })
    }
}

impl From<&common_viss::misc_filter::ParameterVariant> for subscription::MiscFilter {
    fn from(v: &common_viss::misc_filter::ParameterVariant) -> Self {
        match &v {
            common_viss::misc_filter::ParameterVariant::TimebasedParameter(v) => v.into(),
            common_viss::misc_filter::ParameterVariant::ChangeParameter(v) => v.into(),
            common_viss::misc_filter::ParameterVariant::RangeParameter(v) => v.into(),
            common_viss::misc_filter::ParameterVariant::CurvelogParameter(v) => v.into(),
        }
    }
}

// Other Converters
impl From<&common_viss::ValueType> for model::Value {
    fn from(v: &common_viss::ValueType) -> model::Value {
        match &v.value_type {
            None => model::Value::Nan,
            Some(common_viss::value_type::ValueType::String(value)) => {
                model::Value::String(value.clone())
            }
            Some(common_viss::value_type::ValueType::Bool(b)) => model::Value::Bool(*b),
            Some(common_viss::value_type::ValueType::Int8(i)) => model::Value::Int8(*i as i8),
            Some(common_viss::value_type::ValueType::Int16(i)) => model::Value::Int16(*i as i16),
            Some(common_viss::value_type::ValueType::Int32(i)) => model::Value::Int32(*i),
            Some(common_viss::value_type::ValueType::Int64(i)) => model::Value::Int64(*i),
            Some(common_viss::value_type::ValueType::Uint8(i)) => model::Value::Uint8(*i as u8),
            Some(common_viss::value_type::ValueType::Uint16(i)) => model::Value::Uint16(*i as u16),
            Some(common_viss::value_type::ValueType::Uint32(u)) => model::Value::Uint32(*u),
            Some(common_viss::value_type::ValueType::Uint64(u)) => model::Value::Uint64(*u),
            Some(common_viss::value_type::ValueType::Float(f)) => model::Value::Float(*f),
            Some(common_viss::value_type::ValueType::Double(d)) => model::Value::Double(*d),
            Some(common_viss::value_type::ValueType::StringArray(string_array)) => {
                model::Value::StringArray(string_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::BoolArray(bool_array)) => {
                model::Value::BoolArray(bool_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::Int8Array(int8_array)) => {
                model::Value::Int8Array(int8_array.values.iter().map(|&x| x as i8).collect())
            }
            Some(common_viss::value_type::ValueType::Int16Array(int16_array)) => {
                model::Value::Int16Array(int16_array.values.iter().map(|&x| x as i16).collect())
            }
            Some(common_viss::value_type::ValueType::Int32Array(int32_array)) => {
                model::Value::Int32Array(int32_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::Int64Array(int64_array)) => {
                model::Value::Int64Array(int64_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::Uint8Array(uint8_array)) => {
                model::Value::Uint8Array(uint8_array.values.iter().map(|&x| x as u8).collect())
            }
            Some(common_viss::value_type::ValueType::Uint16Array(uint16_array)) => {
                model::Value::Uint16Array(uint16_array.values.iter().map(|&x| x as u16).collect())
            }
            Some(common_viss::value_type::ValueType::Uint32Array(uint32_array)) => {
                model::Value::Uint32Array(uint32_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::Uint64Array(uint64_array)) => {
                model::Value::Uint64Array(uint64_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::FloatArray(float_array)) => {
                model::Value::FloatArray(float_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::DoubleArray(double_array)) => {
                model::Value::DoubleArray(double_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::ValueObject(value_object)) => {
                let mut map = std::collections::HashMap::new();
                for property in &value_object.properties {
                    let value = property
                        .value
                        .as_ref()
                        .map(model::Value::from)
                        .unwrap_or(model::Value::Nan);
                    map.insert(property.key.clone(), value);
                }
                model::Value::Complex(map)
            }
            Some(common_viss::value_type::ValueType::ValueObjectArray(value_object_array)) => {
                let arr = value_object_array
                    .values
                    .iter()
                    .map(|value_object| {
                        let mut map = std::collections::HashMap::new();
                        for property in &value_object.properties {
                            let value = property
                                .value
                                .as_ref()
                                .map(model::Value::from)
                                .unwrap_or(model::Value::Nan);
                            map.insert(property.key.clone(), value);
                        }
                        map
                    })
                    .collect();
                model::Value::ComplexArray(arr)
            }
        }
    }
}

impl TryFrom<&model::SignalStateView> for common_viss::DataPackageCurrent {
    type Error = result::KuraError;
    fn try_from(v: &model::SignalStateView) -> Result<Self, Self::Error> {
        if v.state.value == model::Value::Nan {
            return Ok(common_viss::DataPackageCurrent {
                path: v.path.clone(),
                dp: [].to_vec(),
            });
        }
        let value = common_viss::value_type::ValueType::try_from(&v.state.value)?;
        let value = Some(value);
        let value = common_viss::ValueType { value_type: value };
        let value = Some(value);

        let datapoint = common_viss::DataPointCurrent {
            stored_timestamp: v.state.stored_timestamp.unwrap_or(0),
            value,
            produced_timestamp: v.state.produced_timestamp,
            is_available_sensor: v.state.is_available_sensor,
            is_available_actuator: v.state.is_available_actuator,
        };
        Ok(common_viss::DataPackageCurrent {
            path: v.path.clone(),
            dp: [datapoint].to_vec(),
        })
    }
}

impl TryFrom<&model::Value> for common_viss::value_type::ValueType {
    type Error = result::KuraError;

    fn try_from(value: &model::Value) -> Result<Self, Self::Error> {
        let value = match value.clone() {
            model::Value::Nan => {
                return result::KuraResult::Err(result::KuraError {
                    number: result::KuraErrorCode::BadRequest,
                    reason: result::KuraErrorReason::InvalidData,
                    description: Some(format!("failed to convert {value}")),
                });
            }
            model::Value::Bool(value) => common_viss::value_type::ValueType::Bool(value),
            model::Value::String(value) => common_viss::value_type::ValueType::String(value),
            model::Value::Int8(value) => common_viss::value_type::ValueType::Int8(value as i32),
            model::Value::Int16(value) => common_viss::value_type::ValueType::Int16(value as i32),
            model::Value::Int32(value) => common_viss::value_type::ValueType::Int32(value),
            model::Value::Int64(value) => common_viss::value_type::ValueType::Int64(value),
            model::Value::Uint8(value) => common_viss::value_type::ValueType::Uint8(value as u32),
            model::Value::Uint16(value) => common_viss::value_type::ValueType::Uint16(value as u32),
            model::Value::Uint32(value) => common_viss::value_type::ValueType::Uint32(value),
            model::Value::Uint64(value) => common_viss::value_type::ValueType::Uint64(value),
            model::Value::Float(value) => common_viss::value_type::ValueType::Float(value),
            model::Value::Double(value) => common_viss::value_type::ValueType::Double(value),
            model::Value::BoolArray(values) => {
                common_viss::value_type::ValueType::BoolArray(common_viss::BoolArray { values })
            }
            model::Value::StringArray(values) => {
                common_viss::value_type::ValueType::StringArray(common_viss::StringArray { values })
            }
            model::Value::Int8Array(values) => {
                common_viss::value_type::ValueType::Int8Array(common_viss::Int8Array {
                    values: values.into_iter().map(|x| x as i32).collect(),
                })
            }
            model::Value::Int16Array(values) => {
                common_viss::value_type::ValueType::Int16Array(common_viss::Int16Array {
                    values: values.into_iter().map(|x| x as i32).collect(),
                })
            }
            model::Value::Int32Array(values) => {
                common_viss::value_type::ValueType::Int32Array(common_viss::Int32Array { values })
            }
            model::Value::Int64Array(values) => {
                common_viss::value_type::ValueType::Int64Array(common_viss::Int64Array { values })
            }
            model::Value::Uint8Array(values) => {
                common_viss::value_type::ValueType::Uint8Array(common_viss::Uint8Array {
                    values: values.into_iter().map(|x| x as u32).collect(),
                })
            }
            model::Value::Uint16Array(values) => {
                common_viss::value_type::ValueType::Uint16Array(common_viss::Uint16Array {
                    values: values.into_iter().map(|x| x as u32).collect(),
                })
            }
            model::Value::Uint32Array(values) => {
                common_viss::value_type::ValueType::Uint32Array(common_viss::Uint32Array { values })
            }
            model::Value::Uint64Array(values) => {
                common_viss::value_type::ValueType::Uint64Array(common_viss::Uint64Array { values })
            }
            model::Value::FloatArray(values) => {
                common_viss::value_type::ValueType::FloatArray(common_viss::FloatArray { values })
            }
            model::Value::DoubleArray(values) => {
                common_viss::value_type::ValueType::DoubleArray(common_viss::DoubleArray { values })
            }
            model::Value::Complex(values) => {
                common_viss::value_type::ValueType::ValueObject(common_viss::ValueObject {
                    properties: values
                        .iter()
                        .map(|(key, value)| common_viss::ValueProperty {
                            key: key.clone(),
                            value: Some(common_viss::ValueType::from(value)),
                        })
                        .collect(),
                })
            }
            model::Value::ComplexArray(values) => {
                common_viss::value_type::ValueType::ValueObjectArray(
                    common_viss::ValueObjectArray {
                        values: values
                            .iter()
                            .map(|entry| common_viss::ValueObject {
                                properties: entry
                                    .iter()
                                    .map(|(key, value)| common_viss::ValueProperty {
                                        key: key.clone(),
                                        value: Some(common_viss::ValueType::from(value)),
                                    })
                                    .collect(),
                            })
                            .collect(),
                    },
                )
            }
        };
        Ok(value)
    }
}

impl TryFrom<&model::SignalStateView> for common_viss::DataPackageTarget {
    type Error = result::KuraError;
    fn try_from(v: &model::SignalStateView) -> Result<Self, Self::Error> {
        if v.state.value == model::Value::Nan {
            return Ok(common_viss::DataPackageTarget {
                path: v.path.clone(),
                dp: [].to_vec(),
            });
        }
        let value = common_viss::value_type::ValueType::try_from(&v.state.value)?;
        let value = Some(value);
        let value = common_viss::ValueType { value_type: value };
        let value = Some(value);

        let datapoint = common_viss::DataPointTarget { value };
        Ok(common_viss::DataPackageTarget {
            path: v.path.clone(),
            dp: [datapoint].to_vec(),
        })
    }
}

fn convert_single_metadata_to_metadata_entry(
    path: &str,
    v: &model::SignalMetadata,
) -> result::KuraResult<common_viss::MetadataEntry> {
    let allowed = v.allowd.as_ref().map(|ret| common_viss::StringArray {
        values: ret.clone(),
    });

    let description = match &v.description {
        Some(v) => v.clone(),
        None => "".to_string(),
    };

    let default = v.default.as_ref().map(From::from);

    let max = v.max.as_ref().map(From::from);

    let min = v.min.as_ref().map(From::from);

    let mut fields = ::prost::alloc::collections::BTreeMap::new();
    if let Some(user_defined_items) = v.user_defined_items.clone() {
        for (key, value) in user_defined_items {
            let value = ::prost_types::Value {
                kind: Some(prost_types::value::Kind::StringValue(value)),
            };
            fields.insert(key, value);
        }
    }
    let custom_metadata = Some(::prost_types::Struct { fields });

    let data_type = common_viss::DataType::from(&v.data_type);
    let datatype_path = match &v.data_type {
        model::ValueType::TypeComplex(datatype_path)
        | model::ValueType::TypeComplexArray(datatype_path) => Some(datatype_path.clone()),
        _ => None,
    };

    let cur = common_viss::MetadataEntry {
        path: path.to_owned(),
        r#type: common_viss::NodeType::from(&v.leaf_type) as i32,
        datatype: data_type as i32,
        datatype_path,
        deprecated: v.deprecation.clone(),
        unit: v.unit.clone(),
        pattern: None, // because this func creates metadata from only one signal.
        max,
        min,
        description,
        comment: v.comment.clone(),
        allowed,
        arraysize: v.array_size,
        default,
        custom_metadata,
    };
    Ok(cur)
}

impl TryFrom<&model::SignalMetadataView> for common_viss::MetadataEntry {
    type Error = result::KuraError;
    fn try_from(v: &model::SignalMetadataView) -> Result<Self, Self::Error> {
        convert_single_metadata_to_metadata_entry(&v.path, &v.metadata)
    }
}

impl TryFrom<(String, &model::PropertyMetadata)> for common_viss::PropertyMetadataEntry {
    type Error = result::KuraError;
    fn try_from(tuple: (String, &model::PropertyMetadata)) -> Result<Self, Self::Error> {
        let (name, metadata) = tuple;
        let allowed = metadata
            .allowd
            .as_ref()
            .map(|items| common_viss::StringArray {
                values: items.clone(),
            });

        let description = metadata.description.clone().unwrap_or_default();
        // Treat Nan as "no default" for struct properties, because optional
        // defaults are frequently omitted in VSS definitions.
        let default = match &metadata.default {
            Some(model::Value::Nan) | None => None,
            Some(default) => Some(common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::try_from(default)?),
            }),
        };

        let max = metadata.max.as_ref().map(From::from);

        let min = metadata.min.as_ref().map(From::from);

        let datatype_path = match &metadata.data_type {
            model::ValueType::TypeComplex(datatype_path)
            | model::ValueType::TypeComplexArray(datatype_path) => Some(datatype_path.clone()),
            _ => None,
        };

        Ok(common_viss::PropertyMetadataEntry {
            name,
            datatype: common_viss::DataType::from(&metadata.data_type) as i32,
            deprecated: metadata.deprecation.clone(),
            unit: metadata.unit.clone(),
            pattern: None,
            max,
            min,
            description,
            comment: metadata.comment.clone(),
            allowed,
            arraysize: metadata.array_size,
            default,
            datatype_path,
            custom_metadata: None,
        })
    }
}

impl TryFrom<&model::VSSUserDefinedType> for common_viss::StructMetadataEntry {
    type Error = result::KuraError;
    fn try_from(v: &model::VSSUserDefinedType) -> Result<Self, Self::Error> {
        let prefix = format!("{}.", v.name);
        let mut properties = Vec::with_capacity(v.children.len());

        for (path, child) in &v.children {
            let name = path.strip_prefix(&prefix).unwrap_or(path).to_string();
            let property = common_viss::PropertyMetadataEntry::try_from((name, child))?;
            properties.push(property);
        }

        Ok(common_viss::StructMetadataEntry {
            path: v.name.clone(),
            deprecated: None,
            description: "".to_string(),
            comment: None,
            properties,
            custom_metadata: None,
        })
    }
}

impl TryFrom<&HashMap<String, model::VSSUserDefinedType>> for common_viss::MetadataEntries {
    type Error = result::KuraError;
    fn try_from(v: &HashMap<String, model::VSSUserDefinedType>) -> Result<Self, Self::Error> {
        let struct_metadata_entries = v
            .values()
            .map(common_viss::StructMetadataEntry::try_from)
            .collect::<Result<Vec<_>, _>>()?;

        Ok(common_viss::MetadataEntries {
            metadata_entries: Vec::new(),
            struct_metadata_entries,
        })
    }
}

impl From<&model::LeafType> for common_viss::NodeType {
    fn from(v: &model::LeafType) -> Self {
        match v {
            model::LeafType::Branch => common_viss::NodeType::NodeBranch,
            model::LeafType::Sensor => common_viss::NodeType::NodeSensor,
            model::LeafType::Attribute => common_viss::NodeType::NodeAttribute,
            model::LeafType::Actuator => common_viss::NodeType::NodeActuator,
            model::LeafType::Property => {
                unreachable!("yoriito viss is not aware of property node.")
            }
        }
    }
}

impl From<&model::ValueType> for common_viss::DataType {
    fn from(v: &model::ValueType) -> Self {
        match v {
            model::ValueType::TypeNAN => common_viss::DataType::DatatypeNone,
            model::ValueType::TypeBool => common_viss::DataType::DatatypeBool,
            model::ValueType::TypeString => common_viss::DataType::DatatypeString,
            model::ValueType::TypeInt8 => common_viss::DataType::DatatypeInt8,
            model::ValueType::TypeInt16 => common_viss::DataType::DatatypeInt16,
            model::ValueType::TypeInt32 => common_viss::DataType::DatatypeInt32,
            model::ValueType::TypeInt64 => common_viss::DataType::DatatypeInt64,
            model::ValueType::TypeUint8 => common_viss::DataType::DatatypeUint8,
            model::ValueType::TypeUint16 => common_viss::DataType::DatatypeUint16,
            model::ValueType::TypeUint32 => common_viss::DataType::DatatypeUint32,
            model::ValueType::TypeUint64 => common_viss::DataType::DatatypeUint64,
            model::ValueType::TypeFloat => common_viss::DataType::DatatypeFloat,
            model::ValueType::TypeDouble => common_viss::DataType::DatatypeDouble,
            model::ValueType::TypeBoolArray => common_viss::DataType::DatatypeArrayBool,
            model::ValueType::TypeStringArray => common_viss::DataType::DatatypeArrayString,
            model::ValueType::TypeInt8Array => common_viss::DataType::DatatypeArrayInt8,
            model::ValueType::TypeInt16Array => common_viss::DataType::DatatypeArrayInt16,
            model::ValueType::TypeInt32Array => common_viss::DataType::DatatypeArrayInt32,
            model::ValueType::TypeInt64Array => common_viss::DataType::DatatypeArrayInt64,
            model::ValueType::TypeUint8Array => common_viss::DataType::DatatypeArrayUint8,
            model::ValueType::TypeUint16Array => common_viss::DataType::DatatypeArrayUint16,
            model::ValueType::TypeUint32Array => common_viss::DataType::DatatypeArrayUint32,
            model::ValueType::TypeUint64Array => common_viss::DataType::DatatypeArrayUint64,
            model::ValueType::TypeFloatArray => common_viss::DataType::DatatypeArrayFloat,
            model::ValueType::TypeDoubleArray => common_viss::DataType::DatatypeArrayDouble,
            model::ValueType::TypeComplex(_) => common_viss::DataType::DatatypeStruct,
            model::ValueType::TypeComplexArray(_) => common_viss::DataType::DatatypeStructArray,
        }
    }
}

impl From<&model::Value> for common_viss::ValueType {
    fn from(v: &model::Value) -> Self {
        common_viss::ValueType {
            value_type: common_viss::value_type::ValueType::try_from(v).ok(),
        }
    }
}

impl From<&common_viss::Operator> for subscription::Opcode {
    fn from(v: &common_viss::Operator) -> Self {
        match v {
            common_viss::Operator::Undefined => subscription::Opcode::Undefined,
            common_viss::Operator::Eq => subscription::Opcode::Equal,
            common_viss::Operator::Ne => subscription::Opcode::NotEqual,
            common_viss::Operator::Gt => subscription::Opcode::GreaterThan,
            common_viss::Operator::Gte => subscription::Opcode::GreaterEqual,
            common_viss::Operator::Lt => subscription::Opcode::LesserThan,
            common_viss::Operator::Lte => subscription::Opcode::LesserEqual,
        }
    }
}

impl From<&common_viss::Condition> for subscription::Combination {
    fn from(v: &common_viss::Condition) -> Self {
        match v {
            common_viss::Condition::Undefined => subscription::Combination::Undefined,
            common_viss::Condition::And => subscription::Combination::And,
            common_viss::Condition::Or => subscription::Combination::Or,
        }
    }
}

impl From<&common_viss::DataPointCurrent> for model::SignalState {
    fn from(v: &common_viss::DataPointCurrent) -> Self {
        let value = v
            .value
            .clone()
            .map(|x| model::Value::from(&x))
            .unwrap_or(model::Value::Nan);

        model::SignalState {
            stored_timestamp: Some(v.stored_timestamp),
            produced_timestamp: v.produced_timestamp,
            value,
            is_available_sensor: v.is_available_sensor,
            is_available_actuator: v.is_available_actuator,
        }
    }
}

impl From<&common_viss::DataPointTarget> for model::SignalState {
    fn from(v: &common_viss::DataPointTarget) -> Self {
        let value = v
            .value
            .clone()
            .map(|x| model::Value::from(&x))
            .unwrap_or(model::Value::Nan);

        // All properties except value are ignored in set requests.
        model::SignalState {
            stored_timestamp: None,
            produced_timestamp: None,
            value,
            is_available_sensor: None,
            is_available_actuator: None,
        }
    }
}

impl From<&result::KuraError> for common_viss::Error {
    fn from(v: &result::KuraError) -> Self {
        common_viss::Error {
            number: v.number as i32,
            reason: v.reason as i32,
            description: v.description.clone(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn inspect_metadata_entry_keeps_typed_min_max() {
        let metadata = model::SignalMetadata {
            leaf_type: model::LeafType::Sensor,
            data_type: model::ValueType::TypeDouble,
            deprecation: None,
            unit: None,
            min: Some(model::Value::Double(-1.5)),
            max: Some(model::Value::Double(42.25)),
            description: Some("engine speed".to_string()),
            comment: None,
            allowd: None,
            default: None,
            array_size: None,
            pattern: None,
            user_defined_items: None,
        };

        let entry = convert_single_metadata_to_metadata_entry("Vehicle.Speed", &metadata)
            .unwrap_or_else(|error| panic!("metadata conversion should succeed: {error}"));

        assert_eq!(
            entry.min,
            Some(common_viss::ValueType::from(&model::Value::Double(-1.5)))
        );
        assert_eq!(
            entry.max,
            Some(common_viss::ValueType::from(&model::Value::Double(42.25)))
        );
    }

    #[test]
    fn property_metadata_entry_keeps_typed_min_max() {
        let metadata = model::PropertyMetadata {
            leaf_type: model::LeafType::Attribute,
            data_type: model::ValueType::TypeUint64,
            deprecation: None,
            unit: None,
            pattern: None,
            min: Some(model::Value::Uint64(10)),
            max: Some(model::Value::Uint64(99)),
            description: Some("trip count".to_string()),
            comment: None,
            allowd: None,
            default: None,
            array_size: None,
            user_defined_items: None,
        };

        let entry = common_viss::PropertyMetadataEntry::try_from(("Count".to_string(), &metadata))
            .unwrap_or_else(|error| panic!("property metadata conversion should succeed: {error}"));

        assert_eq!(
            entry.min,
            Some(common_viss::ValueType::from(&model::Value::Uint64(10)))
        );
        assert_eq!(
            entry.max,
            Some(common_viss::ValueType::from(&model::Value::Uint64(99)))
        );
    }
}
