# -*- coding: utf-8 -*-
"""Remove duplicate quest section from Usage.md."""
from pathlib import Path

USAGE = Path(__file__).resolve().parents[1] / "Docs" / "Usage.md"
FOOTER = """

---

任务系统（Quest Task、BP_QuestHost、联机）见 [QuestSystemGuide.md](./QuestSystemGuide.md)。
"""

text = USAGE.read_text(encoding="utf-8", errors="replace")
for marker in ("## 任务系统速查", "## \u4efb\u52a1\u7cfb\u7edf\u901f\u67e5", "QuestSystemGuide.md"):
    if marker in text and marker != "QuestSystemGuide.md":
        text = text.split(marker)[0].rstrip()
        break
elif "QuestSystemGuide.md" in text:
    text = text.split("---")[0].rstrip()
    # keep content up to last code block closing - too fragile; skip if already trimmed

while text.endswith("---"):
    text = text.rstrip().removesuffix("---").rstrip()

USAGE.write_text(text + FOOTER, encoding="utf-8", newline="\n")
print("Usage.md: trimmed")
