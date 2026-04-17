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

from textual.widget import Widget
from textual.widgets import Input
from textual.app import ComposeResult
from textual_autocomplete import AutoComplete

from typing import List


class PathCompleter(Widget):

    def __init__(self, id: str, help: str, candidates: List[str]) -> None:
        super().__init__()
        self.id: str = id
        self.help: str = help
        self.candidates: List[str] = candidates

    def compose(self) -> ComposeResult:
        self.path_input = Input(id=self.id, placeholder=self.help)
        yield self.path_input
        yield AutoComplete(
            self.path_input,
            candidates=self.candidates
        )

    def get_text(self) -> str:
        return self.path_input.value
