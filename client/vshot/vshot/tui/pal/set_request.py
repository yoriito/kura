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
from vshot.client.pal_client import PALClient
from typing import List


class SetRequestBuilder(Widget):

    _ID_PATH: str = "set_path"
    _ID_VALUE: str = "set_value"
    _ID_SEND_BUTTON: str = "set_send_button"
    _ID_RESET_BUTTON: str = "set_reset_button"
    _ID_RESPONSE: str = "set_response"
    _CLASS_CONTAINER: str = "set_container"
    _CLASS_HORIZONTAL_SPACER: str = "set_horizontal_spacer"

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
    #{_ID_VALUE} {{
        width: auto;
        height: auto;
        min-width: 80;
        border: none;
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
        border: none;
        min-width: 0;
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

    def __init__(self, client: PALClient, log: Log) -> None:
        super().__init__()
        self.client: PALClient = client
        self.value: str = ""
        self.logger: Log = log

        # Widgets
        path_input_widget_help: str = "Target vss path. ex) Vehicle"
        self.path_input_widget: PathCompleter = PathCompleter(
            id=SetRequestBuilder._ID_PATH, help=path_input_widget_help, candidates=self.client.get_all_paths())
        self.path_input_widget.tooltip = path_input_widget_help

        value_input_help: str = "Value to target path. For struct, pass JSON object. ex) {\"foo\":1}"
        self.value_input_widget: Input = Input(
            id=SetRequestBuilder._ID_VALUE, placeholder=value_input_help)
        self.value_input_widget.tooltip = value_input_help

        self.send_button_widget: Button = Button(
            "👉 set", id=SetRequestBuilder._ID_SEND_BUTTON)
        self.response_widget: TextArea = TextArea(
            id=SetRequestBuilder._ID_RESPONSE)
        self.reset_button_widget: Button = Button(
            "👉 clear", id=SetRequestBuilder._ID_RESET_BUTTON)

    def compose(self) -> ComposeResult:
        yield Static("set request:")
        with Horizontal(classes=SetRequestBuilder._CLASS_CONTAINER):
            yield self.path_input_widget
            yield self._put_horizontal_spacer()
            yield self.value_input_widget
            yield self._put_horizontal_spacer()
            yield self.send_button_widget
        with Horizontal(classes=SetRequestBuilder._CLASS_CONTAINER):
            yield Static("set response:")
            yield self._put_horizontal_spacer()
            yield self.reset_button_widget
        yield self.response_widget

    def on_input_changed(self, event: Input.Changed) -> None:
        if event.input.id == SetRequestBuilder._ID_PATH:
            # self.path = event.value
            # use PathCompleter.get_text.
            # textual_autocomplete.AutoComplete(=backend of PathCompleter) suppresses Input.Changed event.
            pass
        elif event.input.id == SetRequestBuilder._ID_VALUE:
            self.value: str = event.value
        else:
            pass

    def on_button_pressed(self, event: Button.Pressed) -> None:
        try:
            if event.button.id == SetRequestBuilder._ID_SEND_BUTTON:
                response: str = self.client.set(
                    self.path_input_widget.get_text(), self.value).__str__()
                self.logger.write_line(response)
                self.response_widget.text = response
            elif event.button.id == SetRequestBuilder._ID_RESET_BUTTON:
                self.response_widget.text = ""
            else:
                pass
        except Exception as e:
            self.logger.write_line(e.__str__())

    def _put_horizontal_spacer(self) -> Static:
        return Static("", classes=SetRequestBuilder._CLASS_HORIZONTAL_SPACER)
