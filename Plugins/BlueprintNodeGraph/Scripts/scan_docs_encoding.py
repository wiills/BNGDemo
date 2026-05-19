# -*- coding: utf-8 -*-
from pathlib import Path

docs = Path(__file__).resolve().parents[1] / "Docs"
for p in sorted(docs.glob("*.md")):
    raw = p.read_bytes()
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError as e:
        print(f"{p.name}: INVALID UTF-8 at {e.start}")
        continue
    bad = sum(1 for c in text if c in "\ufffd" or (ord(c) > 127 and "ä»" in text[:500]))
    mojibake_markers = ("Ã©", "Ã¤", "ä»»å", "åŠ¡ç", "ç³»ç")
    mojibake = any(s in text for s in mojibake_markers)
    non_ascii = sum(1 for c in text if ord(c) > 127)
    if "\ufffd" in text:
        print(f"{p.name}: contains U+FFFD replacement char")
    elif mojibake:
        print(f"{p.name}: likely mojibake (non_ascii={non_ascii})")
    else:
        print(f"{p.name}: valid UTF-8, non_ascii={non_ascii}")
