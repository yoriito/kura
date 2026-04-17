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
from textual.widget import Widget
from textual.app import ComposeResult
from textual.widgets import Input, Static, Button, TextArea, Log
from textual.containers import Horizontal, Center, Vertical
from textual.timer import Timer

from vshot.path_completer import PathCompleter
from vshot.client.pal_client import PALClient
from typing import List, Optional, Iterable, Any
import threading
import queue


class SubscribeRequestBuilder(Widget):

    _ID_PATH: str = "subscribe_path"
    _ID_PATHS_FILTER: str = "subscribe_paths_filter"
    _ID_TIMEBASED_FILTER: str = "subscribe_timebased_filter"
    _ID_SEND_BUTTON: str = "subscribe_send_button"
    _ID_RESET_BUTTON: str = "subscribe_reset_button"
    _ID_RESPONSE: str = "subscribe_response"
    _CLASS_CONTAINER: str = "subscribe_container"
    _CLASS_HORIZONTAL_SPACER: str = "subscribe_horizontal_spacer"
    _CLASS_VERTICAL_SPACER: str = "subscribe_vertical_spacer"

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
    #{_ID_TIMEBASED_FILTER} {{
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
    .{_CLASS_VERTICAL_SPACER} {{
        height: 1;
    }}
    """

    def __init__(self, client: PALClient, log: Log) -> None:
        super().__init__()
        self.client: PALClient = client
        self.paths_filter: List[str] = []
        self.timebased_filter: Optional[int] = None
        self.logger: Log = log
        self.response: Optional[Iterable] = None

        # Widgets
        path_input_widget_help: str = "Target vss path. ex) Vehicle"
        self.path_input_widget: PathCompleter = PathCompleter(
            id=SubscribeRequestBuilder._ID_PATH, help=path_input_widget_help, candidates=self.client.get_all_paths())
        self.path_input_widget.tooltip = path_input_widget_help

        paths_filter_input_help: str = "Paths filter. ex) Speed,Body"
        self.paths_filter_input_widget: Input = Input(
            id=SubscribeRequestBuilder._ID_PATHS_FILTER, placeholder=paths_filter_input_help)
        self.paths_filter_input_widget.tooltip = paths_filter_input_help

        timebased_filter_input_help: str = "Timebased filter interval(milliseconds). ex) 5"
        self.timebased_filter_input_widget: Input = Input(
            id=SubscribeRequestBuilder._ID_TIMEBASED_FILTER, placeholder=timebased_filter_input_help)
        self.timebased_filter_input_widget.tooltip = timebased_filter_input_help

        self.send_button_widget: Button = Button(
            "👉 subscribe", id=SubscribeRequestBuilder._ID_SEND_BUTTON)
        self.response_widget: TextArea = TextArea(
            id=SubscribeRequestBuilder._ID_RESPONSE)
        self.reset_button_widget: Button = Button(
            "👉 clear", id=SubscribeRequestBuilder._ID_RESET_BUTTON)

        self._recieve_thread = threading.Thread(target=self._recieve)
        self._recieve_thread.start()

    def compose(self) -> ComposeResult:
        yield Static("subscribe request:")
        with Horizontal(classes=SubscribeRequestBuilder._CLASS_CONTAINER):
            yield self.path_input_widget
            yield self._put_horizontal_spacer()
            yield self.paths_filter_input_widget
            yield self._put_horizontal_spacer()
            yield self.send_button_widget
        yield self._put_vertical_spacer()
        yield self.timebased_filter_input_widget
        yield self._put_vertical_spacer()
        with Horizontal(classes=SubscribeRequestBuilder._CLASS_CONTAINER):
            yield Static("subscribe response:")
            yield self._put_horizontal_spacer()
            yield self.reset_button_widget
        yield self.response_widget

    def on_input_changed(self, event: Input.Changed) -> None:
        if event.input.id == SubscribeRequestBuilder._ID_PATH:
            # self.path = event.value
            # use PathCompleter.get_text.
            # textual_autocomplete.AutoComplete(=backend of PathCompleter) suppresses Input.Changed event.
            pass
        elif event.input.id == SubscribeRequestBuilder._ID_PATHS_FILTER:
            self.paths_filter = event.value.split(",") if event.value else None
        elif event.input.id == SubscribeRequestBuilder._ID_TIMEBASED_FILTER:
            try:
                self.timebased_filter = int(event.value)
            except Exception as e:
                self.timebased_filter = None
                self.logger.write_line(e.__str__())
        else:
            pass

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == SubscribeRequestBuilder._ID_SEND_BUTTON:
            self.response: Iterable = self.client.subscribe(self.path_input_widget.get_text(
            ), self.paths_filter, period=self.timebased_filter, operator=None, diff=None)
        elif event.button.id == SubscribeRequestBuilder._ID_RESET_BUTTON:
            self.response_widget.text = ""
        else:
            pass

    def on_shudown(self):
        self.shutdown = True

    def _put_horizontal_spacer(self) -> Static:
        return Static("", classes=SubscribeRequestBuilder._CLASS_HORIZONTAL_SPACER)

    def _put_vertical_spacer(self) -> Static:
        return Static("", classes=SubscribeRequestBuilder._CLASS_VERTICAL_SPACER)

    def _recieve(self):
        while self.shutdown == False:
            time.sleep(0.03)
            if self.response is None:
                self.logger.write_line("No events")
                continue
            self.logger.write_line("get events!")
            event = next(self.response)
            self.logger.write_line(event .__str__())
