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

import threading
from concurrent.futures import ThreadPoolExecutor, Future
from typing import Callable, Any, Iterable, Optional, List
from termcolor import cprint

CancelFn = Callable[[], None]


class SubscribeRepository:
    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._stop = threading.Event()
        self._executor: Optional[ThreadPoolExecutor] = None
        self._futures: List[Future] = []

    def start(self, max_workers: int = 10) -> None:
        with self._lock:
            if self._executor is None:
                self._executor = ThreadPoolExecutor(max_workers=max_workers)

    def add(self, subscription: Iterable) -> None:
        ex = self._executor
        if ex is None:
            raise RuntimeError("start() must be called before add().")
        with self._lock:
            self._futures.append(ex.submit(self._receive_loop, subscription))

    def stop(self) -> None:
        self._stop.set()
        with self._lock:
            ex = self._executor
            self._executor = None
        if ex:
            ex.shutdown(wait=False)

    def _receive_loop(self, subscription: Iterable) -> None:
        try:
            for msg in subscription:
                if self._stop.is_set():
                    break
                cprint(f"{msg}", "red")
        except Exception as e:
            print(f"receive failed: {e}")
