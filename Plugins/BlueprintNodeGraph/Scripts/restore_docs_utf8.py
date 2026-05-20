# -*- coding: utf-8 -*-
"""Restore Docs/*.md as UTF-8 from git or rewrite quest docs."""
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DOCS = Path(__file__).resolve().parents[1] / "Docs"
REPO = ROOT


def git_bytes(rev: str, path: str) -> bytes:
    return subprocess.check_output(["git", "show", f"{rev}:{path}"], cwd=REPO)


def write_utf8(path: Path, text: str) -> None:
    path.write_text(text, encoding="utf-8", newline="\n")


def restore_from_git(name: str, rev: str = "ef95dec") -> bool:
    rel = f"Plugins/BlueprintNodeGraph/Docs/{name}"
    raw = git_bytes(rev, rel)
    try:
        text = raw.decode("utf-8")
    except UnicodeDecodeError:
        try:
            text = raw.decode("gbk")
        except UnicodeDecodeError:
            print(f"{name}: cannot decode from {rev}")
            return False
    write_utf8(DOCS / name, text)
    print(f"{name}: restored from {rev} ({len(text)} chars)")
    return True


DOCS_README = """# Docs 文档目录

## 编码约定（必读）

本目录下所有 `.md` 文件必须为 **UTF-8** 编码（推荐 **UTF-8 无 BOM**；Windows 记事本可选用 UTF-8 BOM）。

### 禁止

- 使用 PowerShell 默认编码对含中文文件做批量 `Set-Content` / 替换（易破坏多字节字符）
- 在 GBK/ANSI 编辑器中保存后再提交
- 混用已损坏的「伪 UTF-8」（mojibake，如 `ä»»åŠ¡`）

### 推荐

- 使用 VS Code / Cursor：右下角编码显示为 **UTF-8**
- 批量修改用 Python 3：`path.write_text(text, encoding="utf-8")`
- 源码内用户可见中文优先 `NSLOCTEXT`；文档可用中文

### 校验

```bash
python Plugins/BlueprintNodeGraph/Scripts/scan_docs_encoding.py
```

全部应输出 `valid UTF-8`。

## 文件索引

| 文件 | 内容 |
|------|------|
| [QuestSystemGuide.md](./QuestSystemGuide.md) | 任务系统 |
| [Usage.md](./Usage.md) | 节点与 Latent 使用 |
| [Architecture.md](./Architecture.md) | 插件架构 |
"""

if __name__ == "__main__":
    restore_from_git("Usage.md")
    restore_from_git("Architecture.md")
    write_utf8(DOCS / "README.md", DOCS_README)
    print("README.md: created")
