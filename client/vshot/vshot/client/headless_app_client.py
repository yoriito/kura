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

import time
import grpc
import socket
import threading
import dataclasses
from distutils.util import strtobool
from enum import Enum
from vshot.yoriito.viss.v1.consumer_pb2_grpc import VISSStub

import vshot.yoriito.viss.v1.consumer_pb2 as consumer
import vshot.yoriito.viss.v1.common_pb2 as common
import vshot.yoriito.viss.v1.types.filter_pb2 as filter
import vshot.yoriito.viss.v1.types.value_pb2 as value

from typing import List, Optional, Iterable, Callable


class Operator(Enum):
    OPERATOR_UNDEFINED = 0
    OPERATOR_EQ = 1
    OPERATOR_NE = 2
    OPERATOR_GT = 3
    OPERATOR_GTE = 4
    OPERATOR_LT = 5
    OPERATOR_LTE = 6


class HeadlessAppClient:

    def __init__(self: "HeadlessAppClient", ip: str, port: int) -> None:
        if not self._is_server_running(ip, port):
            exit(f"Yoriito VISS server({ip}:{port}) is not running.")
            return
        endpoint: str = f"{ip}:{port}"
        self.channel = grpc.insecure_channel(endpoint)
        self.viss_stub = VISSStub(self.channel)
        self.metadatas: dict[str, _Metadata] = {}

        request: common.InspectRequest = common.InspectRequest(path="*")
        response: common.InspectResponse = self.viss_stub.Inspect(request)
        for single_metadata in response.metadata.entries:
            self.metadatas[single_metadata.path] = _Metadata(
                single_metadata.datatype)

    def get(self: "HeadlessAppClient", path: str, paths_filter: List[str]) -> consumer.GetCurrentResponse:
        paths_filter: filter.PathsFilter = filter.PathsFilter(
            paths=paths_filter)
        get_filter: filter.GetFilter = filter.GetFilter(
            paths_parameter=paths_filter)
        request: consumer.GetCurrentRequest = consumer.GetCurrentRequest(
            path=path, filter=get_filter)
        return self.viss_stub.GetCurrent(request)

    def set(self: "HeadlessAppClient", path: str, _value: str) -> consumer.SetTargetResponse:
        converted_value: value.ValueType = _convert_value(
            _value, self.metadatas[path])
        data_point = value.DataPointTarget(value=converted_value)
        data_package = value.DataPackageTarget(path=path, dp=[data_point])
        data_packages = value.DataPackagesTarget(data=[data_package])
        request: consumer.SetTargetRequest = consumer.SetTargetRequest(
            data=data_packages)
        return self.viss_stub.SetTarget(request)

    def inspect(self: "HeadlessAppClient", path: str, paths_filter: List[str]) -> common.InspectResponse:
        paths_filter: filter.PathsFilter = filter.PathsFilter(
            paths=paths_filter)
        inspect_filter: filter.InspectFilter = filter.InspectFilter(
            paths_parameter=paths_filter)
        request: common.InspectRequest = common.InspectRequest(
            path=path, filter=inspect_filter)
        return self.viss_stub.Inspect(request)

    def subscribe(self: "HeadlessAppClient", path: str, paths_filter: List[str], period: Optional[int], operator: Optional[Operator], diff: Optional[int]) -> Iterable[consumer.SubscribeCurrentResponse]:
        # TODO: Range Filter
        change_filter: Optional[filter.ChangeFilter] = None
        if operator is not None and diff is not None:
            _operator: Optional[filter.Operator] = _convert_operator(operator)
            change_filter = filter.ChangeFilter(logic_op=_operator, diff=diff)
        timebased_filter: Optional[filter.TimebasedFilter] = None
        if period is not None:
            timebased_filter = filter.TimebasedFilter(period=period)
        misc_filter: Optional[filter.MiscFilter] = None
        if change_filter is not None or timebased_filter is not None:
            misc_filter = filter.MiscFilter(
                change_parameter=change_filter, timebased_parameter=timebased_filter)
        paths_filter: filter.PathsFilter = filter.PathsFilter(
            paths=paths_filter)
        subscribe_filter: filter.SubscribeFilter = filter.SubscribeFilter(
            paths_parameter=paths_filter, misc_parameter=misc_filter)
        request: consumer.SubscribeCurrentRequest = consumer.SubscribeCurrentRequest(
            path=path, filter=subscribe_filter)
        return self.viss_stub.SubscribeCurrent(request)

    def unsubscribe(self: "HeadlessAppClient", subscription_id: str) -> common.UnsubscribeResponse:
        request: common.UnsubscribeRequest = common.UnsubscribeRequest(
            subscription_id=subscription_id)
        return self.viss_stub.Unsubscribe(request)

    def get_all_paths(self: "HeadlessAppClient") -> List[str]:
        return list(self.metadatas.keys())

    def _is_server_running(self: "HeadlessAppClient", ip: str, port: int) -> bool:
        try:
            with socket.create_connection((ip, port), 3.0):
                return True
        except OSError:
            return False


@dataclasses.dataclass
class _Metadata:
    data_type: value.DataType


def _convert_operator(operator: Optional[Operator]) -> Optional["filter.Operator"]:
    if operator == None:
        return None
    if operator == Operator.OPERATOR_UNDEFINED:
        return filter.OPERATOR_UNDEFINED
    if operator == Operator.OPERATOR_EQ:
        return filter.OPERATOR_EQ
    if operator == Operator.OPERATOR_NE:
        return filter.OPERATOR_NE
    if operator == Operator.OPERATOR_GT:
        return filter.OPERATOR_GT
    if operator == Operator.OPERATOR_GTE:
        return filter.OPERATOR_GTE
    if operator == Operator.OPERATOR_LT:
        return filter.OPERATOR_LT
    if operator == Operator.OPERATOR_LTE:
        return filter.OPERATOR_LTE
    else:
        return None


def _convert_value(_value: str, metadata: _Metadata) -> value.ValueType:
    data_type: str = metadata.data_type
    if data_type == value.DataType.DATATYPE_NONE:
        return value.ValueType()
    elif data_type == value.DataType.DATATYPE_BOOL:
        return value.ValueType(bool=bool(strtobool(_value)))
    elif data_type == value.DataType.DATATYPE_STRING:
        return value.ValueType(string=_value)
    elif data_type == value.DataType.DATATYPE_INT8:
        return value.ValueType(int32=int(_value))
    elif data_type == value.DataType.DATATYPE_INT16:
        return value.ValueType(int32=int(_value))
    elif data_type == value.DataType.DATATYPE_INT32:
        return value.ValueType(int32=int(_value))
    elif data_type == value.DataType.DATATYPE_INT64:
        return value.ValueType(int64=int(_value))
    elif data_type == value.DataType.DATATYPE_UINT8:
        return value.ValueType(uint8=int(_value))
    elif data_type == value.DataType.DATATYPE_UINT16:
        return value.ValueType(uint16=int(_value))
    elif data_type == value.DataType.DATATYPE_UINT32:
        return value.ValueType(uint32=int(_value))
    elif data_type == value.DataType.DATATYPE_UINT64:
        return value.ValueType(uint64=int(_value))
    elif data_type == value.DataType.DATATYPE_FLOAT:
        return value.ValueType(float=float(_value))
    elif data_type == value.DataType.DATATYPE_DOUBLE:
        return value.ValueType(double=float(_value))
    elif data_type == value.DataType.DATATYPE_ARRAY_BOOL:
        # TODO
        return
    elif data_type == value.DataType.DATATYPE_ARRAY_STRING:
        # TODO
        pass
    elif data_type == value.DataType.DATATYPE_ARRAY_INT8:
        # TODO
        pass
    elif data_type == value.DataType.DATATYPE_ARRAY_INT16:
        # TODO
        pass
    elif data_type == value.DataType.DATATYPE_ARRAY_INT32:
        # TODO
        pass
    elif data_type == value.DataType.DATATYPE_ARRAY_INT64:
        # TODO
        pass
    elif data_type == value.DataType.DATATYPE_ARRAY_UINT8:
        # TODO
        pass
    elif data_type == value.DataType.DATATYPE_ARRAY_UINT16:
        # TODO
        pass
    elif data_type == value.DataType.DATATYPE_ARRAY_UINT32:
        # TODO
        pass
    elif data_type == value.DataType.DATATYPE_ARRAY_UINT64:
        # TODO
        pass
    elif data_type == value.DataType.DATATYPE_ARRAY_FLOAT:
        # TODO
        pass
    elif data_type == value.DataType.DATATYPE_ARRAY_DOUBLE:
        # TODO
        pass
    else:
        pass
