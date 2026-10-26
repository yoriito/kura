# SPDX-License-Identifier: Apache-2.0
#
# Copyright 2026 JASPAR
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import dataclasses
import json
from distutils.util import strtobool
from typing import Any, Optional

import vshot.yoriito.viss.v1.common_pb2 as common
import vshot.yoriito.viss.v1.types.value_pb2 as value


@dataclasses.dataclass
class Metadata:
    data_type: value.DataType
    datatype_path: Optional[str] = None


@dataclasses.dataclass
class MetadataRepository:
    signals: dict[str, Metadata]
    structs: dict[str, dict[str, Metadata]]

    def merge_from_inspect_response(self, response: common.InspectResponse) -> None:
        if response.WhichOneof("result") != "metadata":
            return

        for single_metadata in response.metadata.metadata_entries:
            self.signals[single_metadata.path] = Metadata(
                data_type=single_metadata.datatype,
                datatype_path=(
                    single_metadata.datatype_path
                    if single_metadata.HasField("datatype_path")
                    else None
                ),
            )

        for struct_metadata in response.metadata.struct_metadata_entries:
            properties: dict[str, Metadata] = {}
            for property_metadata in struct_metadata.properties:
                properties[property_metadata.name] = Metadata(
                    data_type=property_metadata.datatype,
                    datatype_path=(
                        property_metadata.datatype_path
                        if property_metadata.HasField("datatype_path")
                        else None
                    ),
                )
            self.structs[struct_metadata.path] = properties

    @classmethod
    def from_inspect_response(
        cls,
        response: common.InspectResponse,
    ) -> "MetadataRepository":
        repo = cls(signals={}, structs={})
        repo.merge_from_inspect_response(response)
        return repo


def convert_value(
    raw_value: str,
    metadata: Metadata,
    metadata_repo: MetadataRepository,
) -> value.ValueType:
    parsed_value = _deserialize_input(raw_value, metadata.data_type)
    return _convert_parsed_value(parsed_value, metadata, metadata_repo)


def _deserialize_input(raw_value: str, data_type: value.DataType) -> Any:
    if data_type == value.DataType.DATATYPE_BOOL:
        return bool(strtobool(raw_value))
    if data_type in _ARRAY_TYPES or data_type == value.DataType.DATATYPE_STRUCT:
        return json.loads(raw_value)
    return raw_value


def _convert_parsed_value(
    parsed_value: Any,
    metadata: Metadata,
    metadata_repo: MetadataRepository,
) -> value.ValueType:
    data_type = metadata.data_type

    if data_type == value.DataType.DATATYPE_NONE:
        return value.ValueType()
    if data_type == value.DataType.DATATYPE_BOOL:
        return value.ValueType(bool=bool(parsed_value))
    if data_type == value.DataType.DATATYPE_STRING:
        return value.ValueType(string=str(parsed_value))
    if data_type == value.DataType.DATATYPE_INT8:
        return value.ValueType(int8=int(parsed_value))
    if data_type == value.DataType.DATATYPE_INT16:
        return value.ValueType(int16=int(parsed_value))
    if data_type == value.DataType.DATATYPE_INT32:
        return value.ValueType(int32=int(parsed_value))
    if data_type == value.DataType.DATATYPE_INT64:
        return value.ValueType(int64=int(parsed_value))
    if data_type == value.DataType.DATATYPE_UINT8:
        return value.ValueType(uint8=int(parsed_value))
    if data_type == value.DataType.DATATYPE_UINT16:
        return value.ValueType(uint16=int(parsed_value))
    if data_type == value.DataType.DATATYPE_UINT32:
        return value.ValueType(uint32=int(parsed_value))
    if data_type == value.DataType.DATATYPE_UINT64:
        return value.ValueType(uint64=int(parsed_value))
    if data_type == value.DataType.DATATYPE_FLOAT:
        return value.ValueType(float=float(parsed_value))
    if data_type == value.DataType.DATATYPE_DOUBLE:
        return value.ValueType(double=float(parsed_value))
    if data_type == value.DataType.DATATYPE_ARRAY_BOOL:
        return value.ValueType(bool_array=value.BoolArray(values=[bool(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_STRING:
        return value.ValueType(string_array=value.StringArray(values=[str(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_INT8:
        return value.ValueType(int8_array=value.Int8Array(values=[int(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_INT16:
        return value.ValueType(int16_array=value.Int16Array(values=[int(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_INT32:
        return value.ValueType(int32_array=value.Int32Array(values=[int(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_INT64:
        return value.ValueType(int64_array=value.Int64Array(values=[int(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_UINT8:
        return value.ValueType(uint8_array=value.Uint8Array(values=[int(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_UINT16:
        return value.ValueType(uint16_array=value.Uint16Array(values=[int(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_UINT32:
        return value.ValueType(uint32_array=value.Uint32Array(values=[int(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_UINT64:
        return value.ValueType(uint64_array=value.Uint64Array(values=[int(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_FLOAT:
        return value.ValueType(float_array=value.FloatArray(values=[float(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_ARRAY_DOUBLE:
        return value.ValueType(double_array=value.DoubleArray(values=[float(item) for item in _expect_list(parsed_value)]))
    if data_type == value.DataType.DATATYPE_STRUCT:
        return _convert_struct(parsed_value, metadata, metadata_repo)

    raise ValueError(f"Unsupported data type: {data_type}")


def _convert_struct(
    parsed_value: Any,
    metadata: Metadata,
    metadata_repo: MetadataRepository,
) -> value.ValueType:
    struct_value = _expect_dict(parsed_value)
    if metadata.datatype_path is None:
        raise ValueError("Struct metadata_path is missing")

    struct_definition = metadata_repo.structs.get(metadata.datatype_path)
    if struct_definition is None:
        raise ValueError(
            f"Struct metadata '{metadata.datatype_path}' is not found")

    properties: list[value.ValueProperty] = []
    for key, item in struct_value.items():
        property_metadata = struct_definition.get(key)
        if property_metadata is None:
            raise ValueError(
                f"Unknown property '{key}' for struct '{metadata.datatype_path}'",
            )
        properties.append(
            value.ValueProperty(
                key=key,
                value=_convert_parsed_value(
                    item, property_metadata, metadata_repo),
            )
        )

    return value.ValueType(value_object=value.ValueObject(properties=properties))


def _expect_list(parsed_value: Any) -> list[Any]:
    if not isinstance(parsed_value, list):
        raise ValueError("Expected a JSON array")
    return parsed_value


def _expect_dict(parsed_value: Any) -> dict[str, Any]:
    if not isinstance(parsed_value, dict):
        raise ValueError("Expected a JSON object")
    return parsed_value


_ARRAY_TYPES = {
    value.DataType.DATATYPE_ARRAY_BOOL,
    value.DataType.DATATYPE_ARRAY_STRING,
    value.DataType.DATATYPE_ARRAY_INT8,
    value.DataType.DATATYPE_ARRAY_INT16,
    value.DataType.DATATYPE_ARRAY_INT32,
    value.DataType.DATATYPE_ARRAY_INT64,
    value.DataType.DATATYPE_ARRAY_UINT8,
    value.DataType.DATATYPE_ARRAY_UINT16,
    value.DataType.DATATYPE_ARRAY_UINT32,
    value.DataType.DATATYPE_ARRAY_UINT64,
    value.DataType.DATATYPE_ARRAY_FLOAT,
    value.DataType.DATATYPE_ARRAY_DOUBLE,
}
