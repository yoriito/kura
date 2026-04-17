#!/usr/bin/env python3
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


import asyncio
from enum import Enum

from vshot.client.headless_app_client import HeadlessAppClient
from textual.app import App
from textual.app import App, ComposeResult, RenderResult
from textual.widgets import Header, Log, Button, Static
from textual.timer import Timer
from textual.containers import Horizontal

import vshot.yoriito.viss.v1.types.filter_pb2 as filter
from vshot.tui.headless_app.get_request import GetRequestBuilder
from vshot.tui.headless_app.set_request import SetRequestBuilder
from vshot.tui.headless_app.inspect_request import InspectRequestBuilder
from vshot.tui.headless_app.subscribe_request import SubscribeRequestBuilder

from typing import Optional


class HeadlessApp(App):
    TITLE = "Yoriito VISS TUI Headless App"
    _ID_CLEAR_BUTTON: str = "headless_app_clear"

    DEFAULT_CSS = f"""
    GetRequestBuilder {{
        width: 100%;
        height: auto;
        border: solid green;
    }}
    SetRequestBuilder {{
        width: 100%;
        height: auto;
        border: solid green;
    }}
    InspectRequestBuilder {{
        width: 100%;
        height: auto;
        border: solid green;
    }}
    SubscribeRequestBuilder {{
        width: 100%;
        height: auto;
        border: solid green;
    }}
    Log {{
        height: auto;
    }}
    #{_ID_CLEAR_BUTTON} {{
        width: auto;
        height: auto;
        min-width: 0;
        border: none;
    }}
    """

    def __init__(self):
        super().__init__()
        ip: str = "localhost"
        port: int = 50052
        self.client: HeadlessAppClient = HeadlessAppClient(ip, port)
        self.logger_widget: Log = Log()

        self.screen_width: int = 0
        self.screen_height: int = 0

        self.car: str = ""
        self.car_widget = Static(self.car)
        self.car_timer: Optional[Timer] = None
        self.theme = "solarized-light"

    def compose(self) -> ComposeResult:
        yield Header()
        yield self.car_widget
        yield GetRequestBuilder(self.client, self.logger_widget)
        yield SetRequestBuilder(self.client, self.logger_widget)
        yield InspectRequestBuilder(self.client, self.logger_widget)
        yield SubscribeRequestBuilder(self.client, self.logger_widget)
        yield Static("log:")
        yield Button("👉 clear", id=HeadlessApp._ID_CLEAR_BUTTON)
        yield self.logger_widget

    def on_ready(self) -> None:
        self.screen_width, self.screen_height = self.size
        self.car = " " * (self.screen_width - 5) + "🚗..."
        self.car_timer: Timer = self.set_interval(0.01, self.update_text)

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == HeadlessApp._ID_CLEAR_BUTTON:
            self.logger_widget.clear()

    def update_text(self):
        self.car = self.car[1:]
        if len(self.car) == 5:
            self.car = ""
            self.car_timer.stop()
        self.car_widget.update(self.car)
