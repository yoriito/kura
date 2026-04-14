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

.PHONY: build 

APP_NAME=vshot
ENTRYPOINT=vshot.main:main

build:
	@mkdir -p build
	@pip install shiv
	@shiv -o build/$(APP_NAME) -e $(ENTRYPOINT) -r requirements.txt -p "/usr/bin/env python3" ./

install:
	@cp build/vshot /usr/local/bin
