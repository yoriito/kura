#!/usr/bin/env bash
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

set -euo pipefail

detect_mode() {
  local mode="x11"
  if grep -qi "microsoft" /proc/sys/kernel/osrelease 2>/dev/null || grep -qi "WSL" /proc/sys/kernel/osrelease 2>/dev/null; then
    mode="wsl"
  elif [ -n "${WAYLAND_DISPLAY-}" ]; then
    mode="wayland"
  else
    mode="x11"
  fi
  echo "$mode"
}

generate_for_wsl() {
    cat > "$AUTO_FILE" <<YAML
services:
  dev:
    environment:
      WAYLAND_DISPLAY: "${WAYLAND_DISPLAY:-wayland-0}"
      XDG_RUNTIME_DIR: "${XDG_RUNTIME_DIR:-/mnt/wslg/runtime-dir}"
      PULSE_SERVER: "${PULSE_SERVER:-/mnt/wslg/PulseServer}"
      DISPLAY: "${DISPLAY:-:0}"
    devices:
      - "/dev/dxg:/dev/dxg"
    volumes:
      - "\${XDG_RUNTIME_DIR:-/mnt/wslg/runtime-dir}:\${XDG_RUNTIME_DIR:-/mnt/wslg/runtime-dir}"
      - "/mnt/wslg:/mnt/wslg"
      - "/usr/lib/wsl:/usr/lib/wsl:ro"
YAML
}

generate_for_linux_wayland() {
    cat > "$AUTO_FILE" <<YAML
services:
  dev:
    environment:
      WAYLAND_DISPLAY: "${WAYLAND_DISPLAY:-wayland-0}"
      XDG_RUNTIME_DIR: "${USER_RUNTIME}"
    devices:
      - "/dev/dri:/dev/dri"
    volumes:
      - "${USER_RUNTIME}:${USER_RUNTIME}"
YAML
}

generate_for_linux_x11() {
    XAUTH_LINE="      - \"${XAUTHORITY:-${HOME}/.Xauthority}:/home/vscode/.Xauthority:ro\""
    cat > "$AUTO_FILE" <<YAML
services:
  dev:
    environment:
      DISPLAY: "${DISPLAY:-:0}"
      XAUTHORITY: "/home/vscode/.Xauthority"
    devices:
      - "/dev/dri:/dev/dri"
    volumes:
      - "/tmp/.X11-unix:/tmp/.X11-unix:ro"
$( [ -n "$XAUTH_LINE" ] && echo "$XAUTH_LINE" || true )
YAML
}

MODE="$(detect_mode)"
UID_HOST=$(id -u)
USER_RUNTIME="/run/user/${UID_HOST}"
AUTO_FILE="$(dirname "$0")/docker-compose.auto.yml"

case "$MODE" in
  wsl)
    generate_for_wsl
    ;;
  wayland)
    generate_for_linux_wayland
    ;;
  x11|*)
    generate_for_linux_x11
    ;;
esac

echo "[gen-compose] wrote ${AUTO_FILE}"
