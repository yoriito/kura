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
use crate::interface::signal as internal_signal;
use crate::interface::subscription as internal_subscription;
use std::convert::TryFrom;
use tracing::error;

// Result Converters
pub trait VissErrorExt<T> {
    fn to_viss_error(self) -> Result<T, Box<tonic::Status>>;
}

impl<T> VissErrorExt<T> for internal_signal::KuraResult<T> {
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
impl TryFrom<&common_viss::InspectFilter> for internal_subscription::PathsFilter {
    type Error = anyhow::Error;
    fn try_from(v: &common_viss::InspectFilter) -> Result<Self, Self::Error> {
        match &v.parameter_variant {
            None => Err(std::io::Error::new(
                std::io::ErrorKind::NotFound,
                "paths filter not found".to_string(),
            )
            .into()),
            Some(common_viss::inspect_filter::ParameterVariant::PathsParameter(paths_filter)) => {
                Ok(internal_subscription::PathsFilter {
                    paths: paths_filter.paths.clone(),
                })
            }
        }
    }
}

impl TryFrom<&common_viss::SubscribeFilter> for internal_subscription::PathsFilter {
    type Error = anyhow::Error;
    fn try_from(v: &common_viss::SubscribeFilter) -> Result<Self, Self::Error> {
        match &v.parameter_variant {
            Some(common_viss::subscribe_filter::ParameterVariant::PathsParameter(paths_filter)) => {
                Ok(internal_subscription::PathsFilter {
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
    fn to_paths_filter(&self) -> Option<internal_subscription::PathsFilter>;
    fn to_misc_filter(&self) -> Option<internal_subscription::MiscFilter>;
}

impl FilterUtil for common_viss::SubscribeFilter {
    fn to_paths_filter(&self) -> Option<internal_subscription::PathsFilter> {
        match &self.parameter_variant.clone()? {
            common_viss::subscribe_filter::ParameterVariant::PathsParameter(v) => {
                Some(internal_subscription::PathsFilter {
                    paths: v.paths.clone(),
                })
            }
            common_viss::subscribe_filter::ParameterVariant::ComboFilter(v) => {
                Some(internal_subscription::PathsFilter {
                    paths: v.paths_parameter.clone()?.paths,
                })
            }
            _ => None,
        }
    }

    fn to_misc_filter(&self) -> Option<internal_subscription::MiscFilter> {
        match &self.parameter_variant.clone()? {
            common_viss::subscribe_filter::ParameterVariant::MiscParameter(v) => Some(
                internal_subscription::MiscFilter::from(&v.parameter_variant.clone()?),
            ),
            common_viss::subscribe_filter::ParameterVariant::ComboFilter(v) => {
                Some(internal_subscription::MiscFilter::from(
                    &v.misc_parameter.clone()?.parameter_variant?,
                ))
            }
            _ => None,
        }
    }
}

impl FilterUtil for common_viss::GetFilter {
    fn to_paths_filter(&self) -> Option<internal_subscription::PathsFilter> {
        match self.parameter_variant.clone()? {
            common_viss::get_filter::ParameterVariant::PathsParameter(v) => {
                Some(internal_subscription::PathsFilter {
                    paths: v.paths.clone(),
                })
            }
            common_viss::get_filter::ParameterVariant::HistoryParameter(_) => None,
        }
    }

    fn to_misc_filter(&self) -> Option<internal_subscription::MiscFilter> {
        match self.parameter_variant.clone()? {
            common_viss::get_filter::ParameterVariant::PathsParameter(_) => None,
            common_viss::get_filter::ParameterVariant::HistoryParameter(v) => {
                Some(internal_subscription::MiscFilter::from(&v))
            }
        }
    }
}

impl FilterUtil for common_viss::InspectFilter {
    fn to_paths_filter(&self) -> Option<internal_subscription::PathsFilter> {
        match self.parameter_variant.clone()? {
            common_viss::inspect_filter::ParameterVariant::PathsParameter(v) => {
                Some(internal_subscription::PathsFilter {
                    paths: v.paths.clone(),
                })
            }
        }
    }

    fn to_misc_filter(&self) -> Option<internal_subscription::MiscFilter> {
        None
    }
}

impl From<&common_viss::PathsFilter> for internal_subscription::PathsFilter {
    fn from(v: &common_viss::PathsFilter) -> Self {
        internal_subscription::PathsFilter {
            paths: v.paths.clone(),
        }
    }
}

impl From<&common_viss::TimebasedFilter> for internal_subscription::MiscFilter {
    fn from(v: &common_viss::TimebasedFilter) -> Self {
        internal_subscription::MiscFilter::Timebased(internal_subscription::TimebasedFilter {
            interval: v.period,
        })
    }
}

impl From<&common_viss::ChangeFilter> for internal_subscription::MiscFilter {
    fn from(v: &common_viss::ChangeFilter) -> Self {
        let operator = match common_viss::Operator::try_from(v.logic_op) {
            Ok(v) => v,
            Err(_) => return internal_subscription::MiscFilter::Invalid,
        };
        internal_subscription::MiscFilter::Change(internal_subscription::ChangeFilter {
            opecode: (&operator).into(),
            offset: v.diff,
        })
    }
}

impl From<&common_viss::RangeFilter> for internal_subscription::MiscFilter {
    fn from(v: &common_viss::RangeFilter) -> Self {
        let mut conditions: Vec<internal_subscription::RangeCondition> = [].to_vec();
        let mut combination = None;
        match &v.range_type {
            Some(common_viss::range_filter::RangeType::Single(v)) => {
                let operator = match common_viss::Operator::try_from(v.boundary_op) {
                    Ok(v) => v,
                    Err(_) => return internal_subscription::MiscFilter::Invalid,
                };
                conditions.push(internal_subscription::RangeCondition {
                    opecode: (&operator).into(),
                    boundary: v.boundary,
                });
            }
            Some(common_viss::range_filter::RangeType::Multiple(v)) => {
                for cur_condition in &v.conditions {
                    if let Some(condition) = cur_condition.combination_op {
                        let condition = match common_viss::Condition::try_from(condition) {
                            Ok(v) => v,
                            Err(_) => return internal_subscription::MiscFilter::Invalid,
                        };
                        combination = Some((&condition).into());
                    }
                    let operator = match common_viss::Operator::try_from(cur_condition.boundary_op)
                    {
                        Ok(v) => v,
                        Err(_) => return internal_subscription::MiscFilter::Invalid,
                    };
                    conditions.push(internal_subscription::RangeCondition {
                        opecode: (&operator).into(),
                        boundary: cur_condition.boundary,
                    });
                }
            }
            None => {}
        }
        internal_subscription::MiscFilter::Range(internal_subscription::RangeFilter {
            conditions,
            combination,
        })
    }
}

impl From<&common_viss::CurvelogFilter> for internal_subscription::MiscFilter {
    fn from(v: &common_viss::CurvelogFilter) -> Self {
        internal_subscription::MiscFilter::Curvelog(internal_subscription::CurvelogFilter {
            maxerr: v.maxerr,
            bufsize: v.bufsize,
        })
    }
}

impl From<&common_viss::HistoryFilter> for internal_subscription::MiscFilter {
    fn from(v: &common_viss::HistoryFilter) -> Self {
        internal_subscription::MiscFilter::History(internal_subscription::HistoryFilter {
            time_period: v.time_period.clone(),
        })
    }
}

impl From<&common_viss::misc_filter::ParameterVariant> for internal_subscription::MiscFilter {
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
impl From<&common_viss::ValueType> for internal_signal::Value {
    fn from(v: &common_viss::ValueType) -> internal_signal::Value {
        match &v.value_type {
            None => internal_signal::Value::Nan,
            Some(common_viss::value_type::ValueType::String(value)) => {
                internal_signal::Value::String(value.clone())
            }
            Some(common_viss::value_type::ValueType::Bool(b)) => internal_signal::Value::Bool(*b),
            Some(common_viss::value_type::ValueType::Int8(i)) => {
                internal_signal::Value::Int8(*i as i8)
            }
            Some(common_viss::value_type::ValueType::Int16(i)) => {
                internal_signal::Value::Int16(*i as i16)
            }
            Some(common_viss::value_type::ValueType::Int32(i)) => internal_signal::Value::Int32(*i),
            Some(common_viss::value_type::ValueType::Int64(i)) => internal_signal::Value::Int64(*i),
            Some(common_viss::value_type::ValueType::Uint8(i)) => {
                internal_signal::Value::Uint8(*i as u8)
            }
            Some(common_viss::value_type::ValueType::Uint16(i)) => {
                internal_signal::Value::Uint16(*i as u16)
            }
            Some(common_viss::value_type::ValueType::Uint32(u)) => {
                internal_signal::Value::Uint32(*u)
            }
            Some(common_viss::value_type::ValueType::Uint64(u)) => {
                internal_signal::Value::Uint64(*u)
            }
            Some(common_viss::value_type::ValueType::Float(f)) => internal_signal::Value::Float(*f),
            Some(common_viss::value_type::ValueType::Double(d)) => {
                internal_signal::Value::Double(*d)
            }
            Some(common_viss::value_type::ValueType::StringArray(string_array)) => {
                internal_signal::Value::StringArray(string_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::BoolArray(bool_array)) => {
                internal_signal::Value::BoolArray(bool_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::Int8Array(int8_array)) => {
                internal_signal::Value::Int8Array(
                    int8_array.values.iter().map(|&x| x as i8).collect(),
                )
            }
            Some(common_viss::value_type::ValueType::Int16Array(int16_array)) => {
                internal_signal::Value::Int16Array(
                    int16_array.values.iter().map(|&x| x as i16).collect(),
                )
            }
            Some(common_viss::value_type::ValueType::Int32Array(int32_array)) => {
                internal_signal::Value::Int32Array(int32_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::Int64Array(int64_array)) => {
                internal_signal::Value::Int64Array(int64_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::Uint8Array(uint8_array)) => {
                internal_signal::Value::Uint8Array(
                    uint8_array.values.iter().map(|&x| x as u8).collect(),
                )
            }
            Some(common_viss::value_type::ValueType::Uint16Array(uint16_array)) => {
                internal_signal::Value::Uint16Array(
                    uint16_array.values.iter().map(|&x| x as u16).collect(),
                )
            }
            Some(common_viss::value_type::ValueType::Uint32Array(uint32_array)) => {
                internal_signal::Value::Uint32Array(uint32_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::Uint64Array(uint64_array)) => {
                internal_signal::Value::Uint64Array(uint64_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::FloatArray(float_array)) => {
                internal_signal::Value::FloatArray(float_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::DoubleArray(double_array)) => {
                internal_signal::Value::DoubleArray(double_array.values.clone())
            }
            Some(common_viss::value_type::ValueType::ValueObject(_value_object)) => {
                internal_signal::Value::Nan
            }
        }
    }
}

impl TryFrom<&internal_signal::Signal> for common_viss::DataPackageCurrent {
    type Error = internal_signal::KuraError;
    fn try_from(v: &internal_signal::Signal) -> Result<Self, Self::Error> {
        let value = match v.current_datapoint.value.clone() {
            internal_signal::Value::Nan => {
                return Ok(common_viss::DataPackageCurrent {
                    path: v.path.clone(),
                    dp: [].to_vec(),
                });
            }
            internal_signal::Value::Bool(value) => common_viss::value_type::ValueType::Bool(value),
            internal_signal::Value::String(value) => {
                common_viss::value_type::ValueType::String(value)
            }
            internal_signal::Value::Int8(value) => {
                common_viss::value_type::ValueType::Int8(value as i32)
            }
            internal_signal::Value::Int16(value) => {
                common_viss::value_type::ValueType::Int16(value as i32)
            }
            internal_signal::Value::Int32(value) => {
                common_viss::value_type::ValueType::Int32(value)
            }
            internal_signal::Value::Int64(value) => {
                common_viss::value_type::ValueType::Int64(value)
            }
            internal_signal::Value::Uint8(value) => {
                common_viss::value_type::ValueType::Uint8(value as u32)
            }
            internal_signal::Value::Uint16(value) => {
                common_viss::value_type::ValueType::Uint16(value as u32)
            }
            internal_signal::Value::Uint32(value) => {
                common_viss::value_type::ValueType::Uint32(value)
            }
            internal_signal::Value::Uint64(value) => {
                common_viss::value_type::ValueType::Uint64(value)
            }
            internal_signal::Value::Float(value) => {
                common_viss::value_type::ValueType::Float(value)
            }
            internal_signal::Value::Double(value) => {
                common_viss::value_type::ValueType::Double(value)
            }
            internal_signal::Value::BoolArray(values) => {
                common_viss::value_type::ValueType::BoolArray(common_viss::BoolArray { values })
            }
            internal_signal::Value::StringArray(values) => {
                common_viss::value_type::ValueType::StringArray(common_viss::StringArray { values })
            }
            internal_signal::Value::Int8Array(values) => {
                common_viss::value_type::ValueType::Int8Array(common_viss::Int8Array {
                    values: values.into_iter().map(|x| x as i32).collect(),
                })
            }
            internal_signal::Value::Int16Array(values) => {
                common_viss::value_type::ValueType::Int16Array(common_viss::Int16Array {
                    values: values.into_iter().map(|x| x as i32).collect(),
                })
            }
            internal_signal::Value::Int32Array(values) => {
                common_viss::value_type::ValueType::Int32Array(common_viss::Int32Array { values })
            }
            internal_signal::Value::Int64Array(values) => {
                common_viss::value_type::ValueType::Int64Array(common_viss::Int64Array { values })
            }
            internal_signal::Value::Uint8Array(values) => {
                common_viss::value_type::ValueType::Uint8Array(common_viss::Uint8Array {
                    values: values.into_iter().map(|x| x as u32).collect(),
                })
            }
            internal_signal::Value::Uint16Array(values) => {
                common_viss::value_type::ValueType::Uint16Array(common_viss::Uint16Array {
                    values: values.into_iter().map(|x| x as u32).collect(),
                })
            }
            internal_signal::Value::Uint32Array(values) => {
                common_viss::value_type::ValueType::Uint32Array(common_viss::Uint32Array { values })
            }
            internal_signal::Value::Uint64Array(values) => {
                common_viss::value_type::ValueType::Uint64Array(common_viss::Uint64Array { values })
            }
            internal_signal::Value::FloatArray(values) => {
                common_viss::value_type::ValueType::FloatArray(common_viss::FloatArray { values })
            }
            internal_signal::Value::DoubleArray(values) => {
                common_viss::value_type::ValueType::DoubleArray(common_viss::DoubleArray { values })
            }
        };

        let value = Some(value);
        let value = common_viss::ValueType { value_type: value };
        let value = Some(value);

        let datapoint = common_viss::DataPointCurrent {
            stored_timestamp: v.current_datapoint.stored_timestamp.unwrap_or(0),
            value,
            produced_timestamp: v.current_datapoint.produced_timestamp,
            is_available_sensor: v.current_datapoint.is_available_sensor,
            is_available_actuator: v.current_datapoint.is_available_actuator,
        };
        Ok(common_viss::DataPackageCurrent {
            path: v.path.clone(),
            dp: [datapoint].to_vec(),
        })
    }
}

impl TryFrom<&internal_signal::Signal> for common_viss::DataPackageTarget {
    type Error = internal_signal::KuraError;
    fn try_from(v: &internal_signal::Signal) -> Result<Self, Self::Error> {
        let (value, _datatype) = match v.target_datapoint.value.clone() {
            internal_signal::Value::Nan => {
                return Ok(common_viss::DataPackageTarget {
                    path: v.path.clone(),
                    dp: [].to_vec(),
                });
            }
            internal_signal::Value::Bool(value) => (
                common_viss::value_type::ValueType::Bool(value),
                common_viss::DataType::DatatypeBool,
            ),
            internal_signal::Value::String(value) => (
                common_viss::value_type::ValueType::String(value),
                common_viss::DataType::DatatypeString,
            ),
            internal_signal::Value::Int8(value) => (
                common_viss::value_type::ValueType::Int8(value as i32),
                common_viss::DataType::DatatypeInt8,
            ),
            internal_signal::Value::Int16(value) => (
                common_viss::value_type::ValueType::Int16(value as i32),
                common_viss::DataType::DatatypeInt16,
            ),
            internal_signal::Value::Int32(value) => (
                common_viss::value_type::ValueType::Int32(value),
                common_viss::DataType::DatatypeInt32,
            ),
            internal_signal::Value::Int64(value) => (
                common_viss::value_type::ValueType::Int64(value),
                common_viss::DataType::DatatypeInt64,
            ),
            internal_signal::Value::Uint8(value) => (
                common_viss::value_type::ValueType::Uint8(value as u32),
                common_viss::DataType::DatatypeUint8,
            ),
            internal_signal::Value::Uint16(value) => (
                common_viss::value_type::ValueType::Uint16(value as u32),
                common_viss::DataType::DatatypeUint16,
            ),
            internal_signal::Value::Uint32(value) => (
                common_viss::value_type::ValueType::Uint32(value),
                common_viss::DataType::DatatypeUint32,
            ),
            internal_signal::Value::Uint64(value) => (
                common_viss::value_type::ValueType::Uint64(value),
                common_viss::DataType::DatatypeUint64,
            ),
            internal_signal::Value::Float(value) => (
                common_viss::value_type::ValueType::Float(value),
                common_viss::DataType::DatatypeFloat,
            ),
            internal_signal::Value::Double(value) => (
                common_viss::value_type::ValueType::Double(value),
                common_viss::DataType::DatatypeDouble,
            ),
            internal_signal::Value::BoolArray(values) => (
                common_viss::value_type::ValueType::BoolArray(common_viss::BoolArray { values }),
                common_viss::DataType::DatatypeArrayBool,
            ),
            internal_signal::Value::StringArray(values) => (
                common_viss::value_type::ValueType::StringArray(common_viss::StringArray {
                    values,
                }),
                common_viss::DataType::DatatypeArrayString,
            ),
            internal_signal::Value::Int8Array(values) => (
                common_viss::value_type::ValueType::Int8Array(common_viss::Int8Array {
                    values: values.into_iter().map(|x| x as i32).collect(),
                }),
                common_viss::DataType::DatatypeArrayInt8,
            ),
            internal_signal::Value::Int16Array(values) => (
                common_viss::value_type::ValueType::Int16Array(common_viss::Int16Array {
                    values: values.into_iter().map(|x| x as i32).collect(),
                }),
                common_viss::DataType::DatatypeArrayInt16,
            ),
            internal_signal::Value::Int32Array(values) => (
                common_viss::value_type::ValueType::Int32Array(common_viss::Int32Array { values }),
                common_viss::DataType::DatatypeArrayInt32,
            ),
            internal_signal::Value::Int64Array(values) => (
                common_viss::value_type::ValueType::Int64Array(common_viss::Int64Array { values }),
                common_viss::DataType::DatatypeArrayInt64,
            ),
            internal_signal::Value::Uint8Array(values) => (
                common_viss::value_type::ValueType::Uint8Array(common_viss::Uint8Array {
                    values: values.into_iter().map(|x| x as u32).collect(),
                }),
                common_viss::DataType::DatatypeArrayUint8,
            ),
            internal_signal::Value::Uint16Array(values) => (
                common_viss::value_type::ValueType::Uint16Array(common_viss::Uint16Array {
                    values: values.into_iter().map(|x| x as u32).collect(),
                }),
                common_viss::DataType::DatatypeArrayUint16,
            ),
            internal_signal::Value::Uint32Array(values) => (
                common_viss::value_type::ValueType::Uint32Array(common_viss::Uint32Array {
                    values,
                }),
                common_viss::DataType::DatatypeArrayUint32,
            ),
            internal_signal::Value::Uint64Array(values) => (
                common_viss::value_type::ValueType::Uint64Array(common_viss::Uint64Array {
                    values,
                }),
                common_viss::DataType::DatatypeArrayUint64,
            ),
            internal_signal::Value::FloatArray(values) => (
                common_viss::value_type::ValueType::FloatArray(common_viss::FloatArray { values }),
                common_viss::DataType::DatatypeArrayFloat,
            ),
            internal_signal::Value::DoubleArray(values) => (
                common_viss::value_type::ValueType::DoubleArray(common_viss::DoubleArray {
                    values,
                }),
                common_viss::DataType::DatatypeArrayDouble,
            ),
        };
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

impl TryFrom<&internal_signal::Signal> for common_viss::MetadataEntry {
    type Error = internal_signal::KuraError;
    fn try_from(v: &internal_signal::Signal) -> Result<Self, Self::Error> {
        let allowed = v
            .config
            .allowd
            .as_ref()
            .map(|ret| common_viss::StringArray {
                values: ret.clone(),
            });

        let description = match &v.config.description {
            Some(v) => v.clone(),
            None => "".to_string(),
        };

        let default = v.config.default.as_ref().map(From::from);

        let max = v.config.max.clone().and_then(|x| match x {
            internal_signal::Value::Int8(v) => Some(v as i32),
            internal_signal::Value::Int16(v) => Some(v as i32),
            internal_signal::Value::Int32(v) => Some(v),
            internal_signal::Value::Int64(v) => Some(v as i32),
            internal_signal::Value::Uint8(v) => Some(v as i32),
            internal_signal::Value::Uint16(v) => Some(v as i32),
            internal_signal::Value::Uint32(v) => Some(v as i32),
            internal_signal::Value::Uint64(v) => Some(v as i32),
            _ => None,
        });

        let min = v.config.min.clone().and_then(|x| match x {
            internal_signal::Value::Int8(v) => Some(v as i32),
            internal_signal::Value::Int16(v) => Some(v as i32),
            internal_signal::Value::Int32(v) => Some(v),
            internal_signal::Value::Int64(v) => Some(v as i32),
            internal_signal::Value::Uint8(v) => Some(v as i32),
            internal_signal::Value::Uint16(v) => Some(v as i32),
            internal_signal::Value::Uint32(v) => Some(v as i32),
            internal_signal::Value::Uint64(v) => Some(v as i32),
            _ => None,
        });

        let mut fields = ::prost::alloc::collections::BTreeMap::new();
        if let Some(user_defined_items) = v.config.user_defined_items.clone() {
            for (key, value) in user_defined_items {
                let value = ::prost_types::Value {
                    kind: Some(prost_types::value::Kind::StringValue(value)),
                };
                fields.insert(key, value);
            }
        }
        let custom_metadata = Some(::prost_types::Struct { fields });

        let cur = common_viss::MetadataEntry {
            path: v.path.clone(),
            r#type: common_viss::NodeType::from(&v.config.leaf_type) as i32,
            datatype: common_viss::DataType::from(&v.config.data_type) as i32,
            deprecated: v.config.deprecation.clone(),
            unit: v.config.unit.clone(),
            pattern: None, // because this func creates metadata from only one signal.
            max,
            min,
            description,
            comment: v.config.comment.clone(),
            allowed,
            arraysize: v.config.array_size,
            default,
            custom_metadata,
        };
        Ok(cur)
    }
}

impl From<&internal_signal::LeafType> for common_viss::NodeType {
    fn from(v: &internal_signal::LeafType) -> Self {
        match v {
            internal_signal::LeafType::Branch => common_viss::NodeType::NodeBranch,
            internal_signal::LeafType::Sensor => common_viss::NodeType::NodeSensor,
            internal_signal::LeafType::Attribute => common_viss::NodeType::NodeAttribute,
            internal_signal::LeafType::Actuator => common_viss::NodeType::NodeActuator,
        }
    }
}

impl From<&internal_signal::ValueType> for common_viss::DataType {
    fn from(v: &internal_signal::ValueType) -> Self {
        match v {
            internal_signal::ValueType::TypeNAN => common_viss::DataType::DatatypeNone,
            internal_signal::ValueType::TypeBool => common_viss::DataType::DatatypeBool,
            internal_signal::ValueType::TypeString => common_viss::DataType::DatatypeString,
            internal_signal::ValueType::TypeInt8 => common_viss::DataType::DatatypeInt8,
            internal_signal::ValueType::TypeInt16 => common_viss::DataType::DatatypeInt16,
            internal_signal::ValueType::TypeInt32 => common_viss::DataType::DatatypeInt32,
            internal_signal::ValueType::TypeInt64 => common_viss::DataType::DatatypeInt64,
            internal_signal::ValueType::TypeUint8 => common_viss::DataType::DatatypeUint8,
            internal_signal::ValueType::TypeUint16 => common_viss::DataType::DatatypeUint16,
            internal_signal::ValueType::TypeUint32 => common_viss::DataType::DatatypeUint32,
            internal_signal::ValueType::TypeUint64 => common_viss::DataType::DatatypeUint64,
            internal_signal::ValueType::TypeFloat => common_viss::DataType::DatatypeFloat,
            internal_signal::ValueType::TypeDouble => common_viss::DataType::DatatypeDouble,
            internal_signal::ValueType::TypeBoolArray => common_viss::DataType::DatatypeArrayBool,
            internal_signal::ValueType::TypeStringArray => {
                common_viss::DataType::DatatypeArrayString
            }
            internal_signal::ValueType::TypeInt8Array => common_viss::DataType::DatatypeArrayInt8,
            internal_signal::ValueType::TypeInt16Array => common_viss::DataType::DatatypeArrayInt16,
            internal_signal::ValueType::TypeInt32Array => common_viss::DataType::DatatypeArrayInt32,
            internal_signal::ValueType::TypeInt64Array => common_viss::DataType::DatatypeArrayInt64,
            internal_signal::ValueType::TypeUint8Array => common_viss::DataType::DatatypeArrayUint8,
            internal_signal::ValueType::TypeUint16Array => {
                common_viss::DataType::DatatypeArrayUint16
            }
            internal_signal::ValueType::TypeUint32Array => {
                common_viss::DataType::DatatypeArrayUint32
            }
            internal_signal::ValueType::TypeUint64Array => {
                common_viss::DataType::DatatypeArrayUint64
            }
            internal_signal::ValueType::TypeFloatArray => common_viss::DataType::DatatypeArrayFloat,
            internal_signal::ValueType::TypeDoubleArray => {
                common_viss::DataType::DatatypeArrayDouble
            }
        }
    }
}

impl From<&internal_signal::Value> for common_viss::ValueType {
    fn from(v: &internal_signal::Value) -> Self {
        match v.clone() {
            internal_signal::Value::Nan => common_viss::ValueType { value_type: None },
            internal_signal::Value::Bool(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Bool(v)),
            },
            internal_signal::Value::String(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::String(v)),
            },
            internal_signal::Value::Int8(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Int32(v.into())),
            },
            internal_signal::Value::Int16(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Int32(v.into())),
            },
            internal_signal::Value::Int32(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Int32(v)),
            },
            internal_signal::Value::Int64(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Int64(v)),
            },
            internal_signal::Value::Uint8(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Uint32(v.into())),
            },
            internal_signal::Value::Uint16(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Uint32(v.into())),
            },
            internal_signal::Value::Uint32(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Uint32(v)),
            },
            internal_signal::Value::Uint64(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Uint64(v)),
            },
            internal_signal::Value::Float(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Float(v)),
            },
            internal_signal::Value::Double(v) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Double(v)),
            },
            internal_signal::Value::BoolArray(items) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::BoolArray(
                    common_viss::BoolArray { values: items },
                )),
            },
            internal_signal::Value::StringArray(items) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::StringArray(
                    common_viss::StringArray { values: items },
                )),
            },
            internal_signal::Value::Int8Array(items) => {
                let items = items.into_iter().map(|x| x as i32).collect();
                common_viss::ValueType {
                    value_type: Some(common_viss::value_type::ValueType::Int8Array(
                        common_viss::Int8Array { values: items },
                    )),
                }
            }
            internal_signal::Value::Int16Array(items) => {
                let items = items.into_iter().map(|x| x as i32).collect();
                common_viss::ValueType {
                    value_type: Some(common_viss::value_type::ValueType::Int16Array(
                        common_viss::Int16Array { values: items },
                    )),
                }
            }
            internal_signal::Value::Int32Array(items) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Int32Array(
                    common_viss::Int32Array { values: items },
                )),
            },
            internal_signal::Value::Int64Array(items) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Int64Array(
                    common_viss::Int64Array { values: items },
                )),
            },
            internal_signal::Value::Uint8Array(items) => {
                let items = items.into_iter().map(|x| x as u32).collect();
                common_viss::ValueType {
                    value_type: Some(common_viss::value_type::ValueType::Uint8Array(
                        common_viss::Uint8Array { values: items },
                    )),
                }
            }
            internal_signal::Value::Uint16Array(items) => {
                let items = items.into_iter().map(|x| x as u32).collect();
                common_viss::ValueType {
                    value_type: Some(common_viss::value_type::ValueType::Uint16Array(
                        common_viss::Uint16Array { values: items },
                    )),
                }
            }
            internal_signal::Value::Uint32Array(items) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Uint32Array(
                    common_viss::Uint32Array { values: items },
                )),
            },
            internal_signal::Value::Uint64Array(items) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::Uint64Array(
                    common_viss::Uint64Array { values: items },
                )),
            },
            internal_signal::Value::FloatArray(items) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::FloatArray(
                    common_viss::FloatArray { values: items },
                )),
            },
            internal_signal::Value::DoubleArray(items) => common_viss::ValueType {
                value_type: Some(common_viss::value_type::ValueType::DoubleArray(
                    common_viss::DoubleArray { values: items },
                )),
            },
        }
    }
}

impl From<&common_viss::Operator> for internal_subscription::Opecode {
    fn from(v: &common_viss::Operator) -> Self {
        match v {
            common_viss::Operator::Undefined => internal_subscription::Opecode::Undefined,
            common_viss::Operator::Eq => internal_subscription::Opecode::Equal,
            common_viss::Operator::Ne => internal_subscription::Opecode::NotEqual,
            common_viss::Operator::Gt => internal_subscription::Opecode::GreaterThan,
            common_viss::Operator::Gte => internal_subscription::Opecode::GreaterEqual,
            common_viss::Operator::Lt => internal_subscription::Opecode::LesserThan,
            common_viss::Operator::Lte => internal_subscription::Opecode::LesserEqual,
        }
    }
}

impl From<&common_viss::Condition> for internal_subscription::Combination {
    fn from(v: &common_viss::Condition) -> Self {
        match v {
            common_viss::Condition::Undefined => internal_subscription::Combination::Undefined,
            common_viss::Condition::And => internal_subscription::Combination::And,
            common_viss::Condition::Or => internal_subscription::Combination::Or,
        }
    }
}

impl From<&common_viss::DataPointCurrent> for internal_signal::DataPoint {
    fn from(v: &common_viss::DataPointCurrent) -> Self {
        let value = v
            .value
            .clone()
            .map(|x| internal_signal::Value::from(&x))
            .unwrap_or(internal_signal::Value::Nan);
        internal_signal::DataPoint {
            stored_timestamp: Some(v.stored_timestamp),
            produced_timestamp: v.produced_timestamp,
            value,
            is_available_sensor: v.is_available_sensor,
            is_available_actuator: v.is_available_actuator,
        }
    }
}

impl From<&common_viss::DataPointTarget> for internal_signal::DataPoint {
    fn from(v: &common_viss::DataPointTarget) -> Self {
        let value = v
            .value
            .clone()
            .map(|x| internal_signal::Value::from(&x))
            .unwrap_or(internal_signal::Value::Nan);
        internal_signal::DataPoint {
            stored_timestamp: None,
            produced_timestamp: None,
            value,
            is_available_sensor: None,
            is_available_actuator: None,
        }
    }
}

impl From<&internal_signal::KuraError> for common_viss::Error {
    fn from(v: &internal_signal::KuraError) -> Self {
        common_viss::Error {
            number: v.number as i32,
            reason: v.reason as i32,
            description: v.description.clone(),
        }
    }
}
