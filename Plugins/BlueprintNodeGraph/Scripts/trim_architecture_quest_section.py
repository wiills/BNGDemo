# -*- coding: utf-8 -*-
"""Replace duplicate quest architecture block with a pointer."""
from pathlib import Path

ARCH = Path(__file__).resolve().parents[1] / "Docs" / "Architecture.md"
POINTER = """

---

## 任务系统

实现类、Rep 数据流、Quest Task 写回路径见 **[QuestSystemGuide.md](./QuestSystemGuide.md)** 与上文模块树中的 `Quest/` 目录。
"""

text = ARCH.read_text(encoding="utf-8", errors="replace")
for marker in ("## 任务系统架构", "## \u4efb\u52a1\u7cfb\u7edf\u67b6\u6784"):
    if marker in text:
        text = text.split(marker)[0].rstrip()
        break

ARCH.write_text(text + POINTER, encoding="utf-8", newline="\n")
print("Architecture.md: quest section replaced with pointer")
