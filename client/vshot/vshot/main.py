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

# autopep8: off
import os
import sys

# to enable importing generated code(yoriito.viss.v1...)
current_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(current_dir)
# to enable running like python3 vshot/main.py
parent_dir = os.path.dirname(current_dir)
sys.path.append(parent_dir)

from typing import Iterable
import click
from click_repl import register_repl, repl
import asyncio
from enum import Enum
from textual.app import App
from textual.app import App, ComposeResult
from textual.widgets import Button, Static
from vshot.tui.headless_app.headless_app import HeadlessApp
from vshot.tui.pal.pal import PAL
from vshot.client.headless_app_client import HeadlessAppClient
from vshot.client.pal_client import PALClient
from vshot.subscrine_handler import SubscribeRepository

# autopep8: on


class ClientMode(Enum):
    HEADLESSAPP = "HeadlessApp"
    PAL = "PAL"


class AppSelector(App):
    _ID_HEADLESS_APP_MODE_BUTTON: str = "root_headless_app"
    _ID_PAL_MODE_BUTTON: str = "selector_pal"
    _CLASS_HORIZONTAL_SPACER: str = "selector_horizontal_spacer"

    DEFAULT_CSS = f"""
    #{_ID_HEADLESS_APP_MODE_BUTTON} {{
        width: auto;
        height: auto;
        min-width: 0;
        border: none;
    }}
    #{_ID_PAL_MODE_BUTTON} {{
        width: auto;
        height: auto;
        min-width: 0;
        border: none;
    }}
    .{_CLASS_HORIZONTAL_SPACER} {{
        width: 2;
    }}
    """

    def __init__(self):
        super().__init__()
        self.mode: ClientMode = ClientMode.HEADLESSAPP

    def compose(self) -> ComposeResult:
        yield Button("👉 Go to Headless Mode", id=AppSelector._ID_HEADLESS_APP_MODE_BUTTON)
        yield self._put_horizontal_spacer()
        yield Button("👉 Go to PAL Mode", id=AppSelector._ID_PAL_MODE_BUTTON)

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == AppSelector._ID_HEADLESS_APP_MODE_BUTTON:
            self.mode = ClientMode.HEADLESSAPP
            self.exit()
        if event.button.id == AppSelector._ID_PAL_MODE_BUTTON:
            self.mode = ClientMode.PAL
            self.exit()

    def _put_horizontal_spacer(self) -> Static:
        return Static("", classes=AppSelector._CLASS_HORIZONTAL_SPACER)


async def asyc_main() -> None:
    selector = AppSelector()
    await selector.run_async()
    if selector.mode == ClientMode.HEADLESSAPP:
        headless_app = HeadlessApp()
        await headless_app.run_async()
    else:
        pal = PAL()
        await pal.run_async()

INTERACTIVE_MODE_KEY: str = "interactive_mode"
HEADLESS_CLIENT_KEY: str = "headless_client"
PAL_CLIENT_KEY: str = "pal_client"
SUBSCRIBE_REPO_KEY: str = "subscrine_repo"


@click.group(invoke_without_command=True, help="a command line tools to read/write vss signals to kura")
@click.option('--interactive', '-i', is_flag=True, help="use interactive mode")
@click.pass_context
def cli(ctx: click.Context, interactive: bool):
    ctx.ensure_object(dict)
    ip: str = "localhost"
    app_port: int = 50052
    pal_port: int = 50053
    ctx.obj[INTERACTIVE_MODE_KEY] = interactive
    ctx.obj[HEADLESS_CLIENT_KEY] = HeadlessAppClient(ip, app_port)
    ctx.obj[PAL_CLIENT_KEY] = PALClient(ip, pal_port)
    ctx.obj[SUBSCRIBE_REPO_KEY] = SubscribeRepository()
    ctx.obj[SUBSCRIBE_REPO_KEY].start()

    if interactive and ctx.invoked_subcommand is None:
        click.echo("--- start interactive mode (:q to exit) ---")
        try:
            repl(ctx)
        finally:
            ctx.obj[HEADLESS_CLIENT_KEY].channel.close()
            ctx.obj[PAL_CLIENT_KEY].channel.close()
            ctx.obj[SUBSCRIBE_REPO_KEY].stop()
    elif ctx.invoked_subcommand is None:
        click.echo(ctx.get_help())


@cli.command()
@click.argument("path")
@click.argument("value")
@click.pass_context
def set_current(ctx: click.Context, path: str, value: str):
    pal_client: PALClient = ctx.obj[PAL_CLIENT_KEY]
    pal_client.set(path, value)


@cli.command()
@click.argument("path")
@click.argument("value")
@click.pass_context
def set_target(ctx: click.Context, path: str, value: str):
    headless_client: HeadlessAppClient = ctx.obj[HEADLESS_CLIENT_KEY]
    headless_client.set(path, value)


@cli.command()
@click.argument("path")
@click.pass_context
def get_current(ctx: click.Context, path: str):
    headless_client: HeadlessAppClient = ctx.obj[HEADLESS_CLIENT_KEY]
    ret: str = headless_client.get(path, [])
    click.echo(f"{ret}")


@cli.command()
@click.argument("path")
@click.pass_context
def get_target(ctx: click.Context, path: str):
    pal_client: PALClient = ctx.obj[PAL_CLIENT_KEY]
    ret: str = pal_client.get(path, [])
    click.echo(f"{ret}")


@cli.command()
@click.argument("path")
@click.pass_context
def subscribe_current(ctx: click.Context, path: str):
    headless_client: HeadlessAppClient = ctx.obj[HEADLESS_CLIENT_KEY]
    ret: Iterable = headless_client.subscribe(path, [], None, None, None)
    subscription_repo: SubscribeRepository = ctx.obj[SUBSCRIBE_REPO_KEY]
    subscription_repo.add(ret)
    click.echo(f"{ret}")


@cli.command()
@click.argument("path")
@click.pass_context
def subscribe_target(ctx: click.Context, path: str):
    pal_client: PALClient = ctx.obj[PAL_CLIENT_KEY]
    ret: Iterable = pal_client.subscribe(path, [], None, None, None)
    subscription_repo: SubscribeRepository = ctx.obj[SUBSCRIBE_REPO_KEY]
    subscription_repo.add(ret)
    click.echo(f"{ret}")


@cli.command()
@click.argument("subscription_id")
@click.pass_context
def unsubscribe_current(ctx: click.Context, subscription_id: str):
    headless_client: HeadlessAppClient = ctx.obj[HEADLESS_CLIENT_KEY]
    ret: common.UnsubscribeResponse = headless_client.unsubscribe(
        subscription_id)
    click.echo(f"{ret}")


@cli.command()
@click.argument("subscription_id")
@click.pass_context
def unsubscribe_target(ctx: click.Context, subscription_id: str):
    pal_client: PALClient = ctx.obj[PAL_CLIENT_KEY]
    ret: common.UnsubscribeResponse = pal_client.unsubscribe(subscription_id)
    click.echo(f"{ret}")


@cli.command()
@click.pass_context
def tui(ctx: click.Context):
    if ctx.obj.get(INTERACTIVE_MODE_KEY):
        click.echo("WARNING: use tui without interactive mode")
        return
    asyncio.run(asyc_main())


def main() -> None:
    cli()


if __name__ == "__main__":
    main()
