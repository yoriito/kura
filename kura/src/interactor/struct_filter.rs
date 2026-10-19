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
use std::collections::{HashMap, HashSet};
use tracing::warn;

fn extract_struct_ref(data_type: &model::ValueType) -> Option<String> {
    match data_type {
        model::ValueType::TypeComplex(path) | model::ValueType::TypeComplexArray(path) => {
            Some(path.clone())
        }
        _ => None,
    }
}

/// Returns only the user defined types referenced by `signals`,
/// including types referenced transitively through struct members.
pub fn filter_referenced_struct_types(
    signals: &[model::SignalMetadataView],
    vss_user_defined_types: &HashMap<String, model::VSSUserDefinedType>,
) -> HashMap<String, model::VSSUserDefinedType> {
    let mut refs_to_visit: Vec<String> = signals
        .iter()
        .filter_map(|signal| extract_struct_ref(&signal.metadata.data_type))
        .collect();
    let mut referenced_structs: HashSet<String> = HashSet::new();

    while let Some(struct_name) = refs_to_visit.pop() {
        if !referenced_structs.insert(struct_name.clone()) {
            continue;
        }

        if let Some(defined_type) = vss_user_defined_types.get(&struct_name) {
            refs_to_visit.extend(
                defined_type
                    .children
                    .values()
                    .filter_map(|property| extract_struct_ref(&property.data_type)),
            );
        } else {
            warn!("struct type '{struct_name}' is referenced but not defined");
        }
    }

    vss_user_defined_types
        .iter()
        .filter(|(name, _)| referenced_structs.contains(*name))
        .map(|(name, defined_type)| (name.clone(), defined_type.clone()))
        .collect()
}

#[cfg(test)]
mod tests {
    use super::*;

    fn signal(path: &str, data_type: model::ValueType) -> model::SignalMetadataView {
        model::SignalMetadataView {
            path: path.to_string(),
            metadata: model::SignalMetadata {
                leaf_type: model::LeafType::Sensor,
                data_type,
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
            },
        }
    }

    fn property(data_type: model::ValueType) -> model::PropertyMetadata {
        model::PropertyMetadata {
            leaf_type: model::LeafType::Property,
            data_type,
            deprecation: None,
            unit: None,
            pattern: None,
            min: None,
            max: None,
            description: None,
            comment: None,
            allowd: None,
            default: None,
            array_size: None,
            user_defined_items: None,
        }
    }

    fn user_defined_type(
        name: &str,
        children: Vec<(&str, model::PropertyMetadata)>,
    ) -> (String, model::VSSUserDefinedType) {
        (
            name.to_string(),
            model::VSSUserDefinedType {
                name: name.to_string(),
                description: None,
                deprecation: None,
                comment: None,
                children: children
                    .into_iter()
                    .map(|(path, metadata)| (format!("{name}.{path}"), metadata))
                    .collect(),
            },
        )
    }

    #[test]
    fn returns_only_types_referenced_by_signals() {
        let types = HashMap::from([
            user_defined_type(
                "Types.DeliveryInfo",
                vec![("Address", property(model::ValueType::TypeString))],
            ),
            user_defined_type(
                "Types.Unrelated",
                vec![("Value", property(model::ValueType::TypeUint8))],
            ),
        ]);
        let signals = vec![signal(
            "Delivery",
            model::ValueType::TypeComplex("Types.DeliveryInfo".to_string()),
        )];

        let filtered = filter_referenced_struct_types(&signals, &types);

        assert_eq!(filtered.len(), 1);
        assert!(filtered.contains_key("Types.DeliveryInfo"));
    }

    #[test]
    fn includes_nested_struct_references_transitively() {
        let types = HashMap::from([
            user_defined_type(
                "Types.DeliveryInfo",
                vec![(
                    "Open",
                    property(model::ValueType::TypeComplex("Types.OpenHours".to_string())),
                )],
            ),
            user_defined_type(
                "Types.OpenHours",
                vec![("Open", property(model::ValueType::TypeUint8))],
            ),
            user_defined_type(
                "Types.Unrelated",
                vec![("Value", property(model::ValueType::TypeUint8))],
            ),
        ]);
        let signals = vec![signal(
            "ReturnAddress",
            model::ValueType::TypeComplex("Types.DeliveryInfo".to_string()),
        )];

        let filtered = filter_referenced_struct_types(&signals, &types);

        assert_eq!(filtered.len(), 2);
        assert!(filtered.contains_key("Types.DeliveryInfo"));
        assert!(filtered.contains_key("Types.OpenHours"));
    }

    #[test]
    fn terminates_on_cyclic_struct_references() {
        let types = HashMap::from([
            user_defined_type(
                "Types.A",
                vec![(
                    "B",
                    property(model::ValueType::TypeComplex("Types.B".to_string())),
                )],
            ),
            user_defined_type(
                "Types.B",
                vec![(
                    "A",
                    property(model::ValueType::TypeComplex("Types.A".to_string())),
                )],
            ),
        ]);
        let signals = vec![signal(
            "Node",
            model::ValueType::TypeComplex("Types.A".to_string()),
        )];

        let filtered = filter_referenced_struct_types(&signals, &types);

        assert_eq!(filtered.len(), 2);
    }

    #[test]
    fn unions_types_referenced_by_multiple_signals() {
        let types = HashMap::from([
            user_defined_type(
                "Types.DeliveryInfo",
                vec![("Address", property(model::ValueType::TypeString))],
            ),
            user_defined_type(
                "Types.OpenHours",
                vec![("Open", property(model::ValueType::TypeUint8))],
            ),
            user_defined_type(
                "Types.Unrelated",
                vec![("Value", property(model::ValueType::TypeUint8))],
            ),
        ]);
        let signals = vec![
            signal(
                "Delivery",
                model::ValueType::TypeComplex("Types.DeliveryInfo".to_string()),
            ),
            signal(
                "Opening",
                model::ValueType::TypeComplex("Types.OpenHours".to_string()),
            ),
        ];

        let filtered = filter_referenced_struct_types(&signals, &types);

        assert_eq!(filtered.len(), 2);
        assert!(filtered.contains_key("Types.DeliveryInfo"));
        assert!(filtered.contains_key("Types.OpenHours"));
    }

    #[test]
    fn skips_references_to_undefined_types() {
        let types = HashMap::from([user_defined_type(
            "Types.DeliveryInfo",
            vec![(
                "Open",
                property(model::ValueType::TypeComplex("Types.Missing".to_string())),
            )],
        )]);
        let signals = vec![signal(
            "Delivery",
            model::ValueType::TypeComplex("Types.DeliveryInfo".to_string()),
        )];

        let filtered = filter_referenced_struct_types(&signals, &types);

        assert_eq!(filtered.len(), 1);
        assert!(filtered.contains_key("Types.DeliveryInfo"));
    }

    #[test]
    fn returns_empty_when_no_signal_uses_struct_type() {
        let types = HashMap::from([user_defined_type(
            "Types.DeliveryInfo",
            vec![("Address", property(model::ValueType::TypeString))],
        )]);
        let signals = vec![signal("Speed", model::ValueType::TypeFloat)];

        let filtered = filter_referenced_struct_types(&signals, &types);

        assert!(filtered.is_empty());
    }
}
