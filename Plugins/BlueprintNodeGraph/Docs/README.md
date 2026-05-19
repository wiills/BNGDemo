# Docs 文档目录

## 编码约定（必读）

本目录下所有 `.md` 文件必须为 **UTF-8** 编码（推荐 **UTF-8 无 BOM**；Windows 记事本可选用 UTF-8 BOM）。

### 禁止

- 使用 PowerShell 默认编码对含中文文件做批量 `Set-Content` / 替换（易破坏多字节字符）
- 在 GBK/ANSI 编辑器中保存后再提交
- 混用已损坏的「伪 UTF-8」（UTF-8 字节被错误解码后的乱码）

### 内容与版权

- 文档与注释中**不要出现**第三方商业游戏、影视等商标或产品名；用通用设计术语描述（如「全局任务 + POI 子任务」）。
- 可参考常见玩法结构，但不要写成「某某游戏式」。

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
| [QuestSystemGuide.md](./QuestSystemGuide.md) | 任务系统使用与术语 |
| [QuestDevPlan.md](./QuestDevPlan.md) | 开发阶段与 P3 待办 |
| [Usage.md](./Usage.md) | 蓝图节点与 Latent Task 速查（要点式） |
| [Architecture.md](./Architecture.md) | 模块与类职责、运行时机制（要点式） |

维护任务系统文档时，优先编辑 **QuestSystemGuide.md** / **QuestDevPlan.md**（已统一为 UTF-8 中文）。
