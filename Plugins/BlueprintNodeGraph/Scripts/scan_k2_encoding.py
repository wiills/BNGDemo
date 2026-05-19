# -*- coding: utf-8 -*-
from pathlib import Path

k2dir = Path(__file__).resolve().parents[1] / "Source/BlueprintNodeGraphEditor"
paths = list((k2dir / "Public/BlueprintTool/K2Nodes").glob("*"))
paths += list((k2dir / "Private/BlueprintTool/K2Nodes").glob("*"))

for p in sorted(paths):
    if p.suffix not in (".h", ".cpp"):
        continue
    raw = p.read_bytes()
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError as e:
        print(f"{p.name}: UTF-8 decode error at {e.start}")
        continue
    issues = []
    if "\ufffd" in text:
        issues.append("U+FFFD replacement")
    # mojibake patterns
    for bad in ("Ã", "ä»", "åŠ", "ç³", "ï¼", "â€", "è¿", "ä»»"):
        if bad in text:
            issues.append(f"mojibake contains {bad!r}")
            break
    # broken strings
    for i, line in enumerate(text.splitlines(), 1):
        if "TEXT(" in line or "NSLOCTEXT" in line:
            if line.count('"') % 2 != 0:
                issues.append(f"line {i}: unbalanced quotes")
        if "?" in line and ("//" in line or "/*" in line) and any(
            x in line for x in ("任", "务", "节", "点", "延", "迟")
        ):
            issues.append(f"line {i}: truncated chinese?")
    non_ascii = sum(1 for c in text if ord(c) > 127)
    if issues or non_ascii > 0:
        print(f"{p.name}: non_ascii={non_ascii} {issues}")

print("scan complete")
