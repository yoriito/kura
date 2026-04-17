# SPDX-License-Identifier: Apache-2.0
#
# Copyright 2026 JASPAR
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import re
from collections import Counter
from pathlib import Path

# input cpplint.log
LOG_PATH = Path("cpplint.log")

if not LOG_PATH.exists():
    raise SystemExit(f"There is no logfile: {LOG_PATH}")

cat_counter = Counter()
total = 0

with LOG_PATH.open(encoding="utf-8", errors="ignore") as f:
    for raw in f:
        line = raw.strip()
        if not line:
            continue
        category = re.search(r'\[([^\]]+)\]', line)
        if not category:
            continue
        cat = category.group(1)
        cat_counter[cat] += 1
        total += 1

# output markdown
md_lines = []
md_lines.append("# cpplint summary\n")
md_lines.append(f"- Total: **{total}**\n")
md_lines.append("## Category list\n")
md_lines.append("| Category | Count |")
md_lines.append("|---|---:|")
for cat, cnt in cat_counter.most_common():
    md_lines.append(f"| `{cat}` | {cnt} |")

Path("summary.md").write_text("\n".join(md_lines), encoding="utf-8")

