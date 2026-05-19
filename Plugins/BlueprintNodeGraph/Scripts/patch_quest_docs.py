# -*- coding: utf-8 -*-
import pathlib
import re

guide = pathlib.Path(__file__).resolve().parents[1] / "Docs" / "QuestSystemGuide.md"
text = guide.read_text(encoding="utf-8", errors="replace")
text = text.replace("ObjectiveId", "ObjectiveTag")

latent_new = """## 与 BlueprintNodeGraph 延迟任务集成

### 通用 Latent（无任务绑定）

使用 **Create Latent Task**（`ExK2Node_LatentTaskObject`）+ `UExLatentTask_Custom` 子类；在 `OnComplete` 中手动调用 Quest API。

### 任务绑定 Latent（推荐）

使用 **Quest Task**（`UExK2Node_QuestTask`），**不要**用 Create Latent Task 创建 `UExLatentTask_Quest` 体系。

1. 蓝图继承 `UExLatentTask_Quest`
2. 配置 `QuestTag`、`ObjectiveTag`（配置 Tag，非实例 ID）
3. Latent 成功完成时自动更新 Quest 进度

```cpp
#include "BlueprintTool/LatentTasks/ExLatentTask_Quest.h"

UExLatentTask_Quest* Task = UExLatentTask_Quest::CreateQuestProxy(
	WorldContextObject, MyQuestLatentClass);
```

### 术语

| 字段 | 含义 |
|------|------|
| `ObjectiveTag` | 目标配置 GameplayTag，**不是** Actor/实例 ID |
| `QuestTag` / `ObjectiveTag` | UExLatentTask_Quest 上的任务与目标 Tag |
"""

pattern = r"## 与 BlueprintNodeGraph 延迟任务集成.*?(?=\n---\n|\n## 蓝图)"
match = re.search(pattern, text, re.S)
if match:
    text = text[: match.start()] + latent_new + text[match.end() :]

text = re.sub(
    r"\| ObjectiveTag \| FGameplayTag \| [^\|]+\|",
    "| ObjectiveTag | FGameplayTag | 目标配置 Tag（非实例 ID） |",
    text,
    count=1,
)
guide.write_text(text, encoding="utf-8")
print("QuestSystemGuide.md updated")
