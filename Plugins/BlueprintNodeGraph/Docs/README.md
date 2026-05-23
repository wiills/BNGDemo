# Docs 文档目录

## 编码约定（必读）

本目录下所有 `.md` 文件必须为 **UTF-8** 编码（推荐 **UTF-8 无 BOM**）.

- 禁止 PowerShell 默认编码批量改写含中文文件
- 批量修改用 Python：`path.write_text(text, encoding="utf-8")`
- 校验：`python Plugins/BlueprintNodeGraph/Scripts/scan_docs_encoding.py`

## 文件索引

| 文件 | 内容 |
|------|------|
| [QuestSystemGuide.md](./QuestSystemGuide.md) | **任务系统**（术语、BNGDemo 上手、Quest Task、联机） |
| [Usage.md](./Usage.md) | 通用 Latent / K2 节点速查 |
| [Architecture.md](./Architecture.md) | 插件模块与核心机制 |

维护任务文档只改 **QuestSystemGuide.md**（生成脚本：`Scripts/write_quest_system_guide.py`）.
