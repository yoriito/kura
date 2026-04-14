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
from textual.app import ComposeResult
from textual.widgets import Input, Static, Button, TextArea, Log
from textual.containers import Horizontal, Center, Vertical

from vshot.path_completer import PathCompleter
from vshot.client.headless_app_client import HeadlessAppClient
from typing import List


class GetRequestBuilder(Widget):

    _ID_PATH: str = "get_path"
    _ID_PATHS_FILTER: str = "get_paths_filter"
    _ID_SEND_BUTTON: str = "get_send_button"
    _ID_RESET_BUTTON: str = "get_reset_button"
    _ID_RESPONSE: str = "get_response"
    _CLASS_CONTAINER: str = "get_container"
    _CLASS_HORIZONTAL_SPACER: str = "get_horizontal_spacer"

    DEFAULT_CSS = f"""
    Static {{
        width: auto;
        height: auto;
        text-style: underline;
        border: none;
    }}
    #{_ID_PATH} {{
        width: auto;
        height: auto;
        min-width: 80;
        border: none;
    }}
    #{_ID_PATH}:focus {{
        width: auto;
        height: 10;
        min-width: 80;
        border: none;
    }}
    #{_ID_PATHS_FILTER} {{
        width: 80;
        height: auto;
        border: none;
        min-width: 0;
    }}
    #{_ID_SEND_BUTTON} {{
        width: auto;
        height: auto;
        border: none;
        min-width: 0;
    }}
    #{_ID_RESET_BUTTON} {{
        width: auto;
        height: auto;
        min-width: 0;
        border: none;
    }}
    #{_ID_RESPONSE} {{
        width: 100%;
        height: auto;
    }}
    .{_CLASS_CONTAINER} {{
        width: auto;
        height: auto;
    }}
    .{_CLASS_HORIZONTAL_SPACER} {{
        width: 2;
    }}
    """

    def __init__(self, client: HeadlessAppClient, log: Log) -> None:
        super().__init__()
        self.client: HeadlessAppClient = client
        self.paths_filter: List[str] = []
        self.logger: Log = log

        # Widgets
        path_input_widget_help: str = "Target vss path. ex) Vehicle"
        self.path_input_widget: PathCompleter = PathCompleter(
            id=GetRequestBuilder._ID_PATH, help=path_input_widget_help, candidates=self.client.get_all_paths())
        self.path_input_widget.tooltip = path_input_widget_help

        paths_filter_input_help: str = "Paths filter. ex) Speed,Body"
        self.paths_filter_input_widget: Input = Input(
            id=GetRequestBuilder._ID_PATHS_FILTER, placeholder=paths_filter_input_help)
        self.paths_filter_input_widget.tooltip = paths_filter_input_help

        self.send_button_widget: Button = Button(
            "👉 get", id=GetRequestBuilder._ID_SEND_BUTTON)
        self.response_widget: TextArea = TextArea(
            id=GetRequestBuilder._ID_RESPONSE)
        self.reset_button_widget: Button = Button(
            "👉 clear", id=GetRequestBuilder._ID_RESET_BUTTON)

    def compose(self) -> ComposeResult:
        yield Static("get request:")
        with Horizontal(classes=GetRequestBuilder._CLASS_CONTAINER):
            yield self.path_input_widget
            yield self._put_horizontal_spacer()
            yield self.paths_filter_input_widget
            yield self._put_horizontal_spacer()
            yield self.send_button_widget
        with Horizontal(classes=GetRequestBuilder._CLASS_CONTAINER):
            yield Static("get response:")
            yield self._put_horizontal_spacer()
            yield self.reset_button_widget
        yield self.response_widget

    def on_input_changed(self, event: Input.Changed) -> None:
        if event.input.id == GetRequestBuilder._ID_PATH:
            # self.path = event.value
            # use PathCompleter.get_text.
            # textual_autocomplete.AutoComplete(=backend of PathCompleter) suppresses Input.Changed event.
            pass
        elif event.input.id == GetRequestBuilder._ID_PATHS_FILTER:
            self.paths_filter = event.value.split(",") if event.value else None
        else:
            pass

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == GetRequestBuilder._ID_SEND_BUTTON:
            response: str = self.client.get(
                self.path_input_widget.get_text(), self.paths_filter).__str__()
            self.logger.write_line(response)
            self.response_widget.text = response
        elif event.button.id == GetRequestBuilder._ID_RESET_BUTTON:
            self.response_widget.text = ""
        else:
            pass

    def _put_horizontal_spacer(self) -> Static:
        return Static("", classes=GetRequestBuilder._CLASS_HORIZONTAL_SPACER)
