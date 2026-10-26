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
from enum import Enum
from vshot.yoriito.viss.v1.producer_pb2_grpc import VISSStub
from google.protobuf.struct_pb2 import Struct

import vshot.yoriito.viss.v1.producer_pb2 as producer
import vshot.yoriito.viss.v1.common_pb2 as common
import vshot.yoriito.viss.v1.types.filter_pb2 as filter
import vshot.yoriito.viss.v1.types.value_pb2 as value
from vshot.client.common import MetadataRepository, convert_value

from typing import List, Optional, Iterable, Callable


class Operator(Enum):
    OPERATOR_UNDEFINED = 0
    OPERATOR_EQ = 1
    OPERATOR_NE = 2
    OPERATOR_GT = 3
    OPERATOR_GTE = 4
    OPERATOR_LT = 5
    OPERATOR_LTE = 6


class PALClient:

    def __init__(self: "PALClient", ip: str, pal_port: int) -> None:
        if not self._is_server_running(ip, pal_port):
            exit(f"Yoriito VISS server({ip}:{pal_port}) is not running.")
            return
        endpoint: str = f"{ip}:{pal_port}"
        self.channel = grpc.insecure_channel(endpoint)
        self.viss_stub = VISSStub(self.channel)

        request: common.InspectRequest = common.InspectRequest(path="*")
        response: common.InspectResponse = self.viss_stub.Inspect(request)
        self.metadata_repo = MetadataRepository.from_inspect_response(response)

    def get(self: "PALClient", path: str, paths_filter: List[str]) -> producer.GetTargetResponse:
        paths_filter: filter.PathsFilter = filter.PathsFilter(
            paths=paths_filter)
        get_filter: filter.GetFilter = filter.GetFilter(
            paths_parameter=paths_filter)
        request: producer.GetTargetRequest = producer.GetTargetRequest(
            path=path, filter=get_filter)
        return self.viss_stub.GetTarget(request)

    def set(self: "PALClient", path: str, _value: str) -> producer.SetCurrentResponse:
        if path not in self.metadata_repo.signals:
            # Refresh metadata for the target path lazily to avoid startup cache misses.
            response = self.inspect(path, [])
            self.metadata_repo.merge_from_inspect_response(response)
        if path not in self.metadata_repo.signals:
            raise KeyError(
                f"Signal metadata not found for path '{path}'. "
                "Check whether the running server loaded this VSS path."
            )
        converted_value: value.ValueType = convert_value(
            _value, self.metadata_repo.signals[path], self.metadata_repo)
        data_point = value.DataPointCurrent(value=converted_value)
        data_package = value.DataPackageCurrent(path=path, dp=[data_point])
        data_packages = value.DataPackagesCurrent(data=[data_package])
        request: producer.SetCurrentRequest = producer.SetCurrentRequest(
            data=data_packages)
        return self.viss_stub.SetCurrent(request)

    def inspect(self: "PALClient", path: str, paths_filter: List[str]) -> common.InspectResponse:
        paths_filter: filter.PathsFilter = filter.PathsFilter(
            paths=paths_filter)
        inspect_filter: filter.InspectFilter = filter.InspectFilter(
            paths_parameter=paths_filter)
        request: common.InspectRequest = common.InspectRequest(
            path=path, filter=inspect_filter)
        return self.viss_stub.Inspect(request)

    def define(self: "PALClient", path: str, paths_filter: List[str], metadata: dict[str, str]) -> producer.DefineResponse:
        paths_filter: filter.PathsFilter = filter.PathsFilter(
            paths=paths_filter)
        define_filter: filter.DefineFilter = filter.DefineFilter(
            paths_parameter=paths_filter)
        _metadata: Struct = Struct()
        _metadata.update(metadata)
        request: producer.DefineRequest = producer.DefineRequest(
            path=path, filter=define_filter, metadata=_metadata)
        return self.viss_stub.Define(request)

    def delete(self: "PALClient", path: str) -> producer.DeleteResponse:
        request: producer.DeleteRequest = producer.DeleteRequest(
            path=[path])
        return self.viss_stub.Delete(request)

    def subscribe(self: "PALClient", path: str, paths_filter: List[str], period: Optional[int], operator: Optional[Operator], diff: Optional[int]) -> Iterable[producer.SubscribeTargetResponse]:
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
        request: producer.SubscribeTargetRequest = producer.SubscribeTargetRequest(
            path=path, filter=subscribe_filter)
        return self.viss_stub.SubscribeTarget(request)

    def unsubscribe(self: "PALClient", subscription_id: str) -> None:
        request: common.UnsubscribeRequest = common.UnsubscribeRequest(
            subscription_id=subscription_id)
        return self.viss_stub.Unsubscribe(request)

    def get_all_paths(self: "PALClient") -> List[str]:
        return list(self.metadata_repo.signals.keys())

    def _is_server_running(self: "PALClient", ip: str, port: int) -> bool:
        try:
            with socket.create_connection((ip, port), 3.0):
                return True
        except OSError:
            return False


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
