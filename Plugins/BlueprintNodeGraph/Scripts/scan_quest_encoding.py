# -*- coding: utf-8 -*-
from pathlib import Path

root = Path(__file__).resolve().parents[1] / "Source/BlueprintNodeGraph"
paths = list((root / "Public/Quest").glob("*")) + list((root / "Private/Quest").glob("*"))
for p in sorted(paths):
    if p.suffix not in (".h", ".cpp"):
        continue
    text = p.read_text(encoding="utf-8", errors="replace")
    issues = []
    if "\ufffd" in text:
        issues.append("replacement char U+FFFD")
    if "?" in text and "TEXT(" in text:
        for line_no, line in enumerate(text.splitlines(), 1):
            if "TEXT(" in line and line.rstrip().endswith("?));"):
                issues.append(f"line {line_no}: truncated TEXT")
    non_ascii = sum(1 for c in text if ord(c) > 127)
    if issues:
        print(p.name, issues)
    elif non_ascii:
        print(p.name, f"has {non_ascii} non-ascii chars (may be ok)")

print("done")
