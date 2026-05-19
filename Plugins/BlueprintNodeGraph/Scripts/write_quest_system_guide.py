# -*- coding: utf-8 -*-
from pathlib import Path

GUIDE = r'''# 任务系统使用指南

> 编码：UTF-8。术语说明见下文「概念与术语」。

## 概述

BlueprintNodeGraph 插件提供层级任务系统，支持：

- **扁平存储 + 父子关系**：`AllTasks` + `ParentTaskId` / `SubTaskIds`
- **GameplayTag**：`TaskId`（任务）与 `ObjectiveTag`（目标）
- **DataAsset**：`UExQuestDataAsset`
- **存档**：JSON `#ExQuestSaveV2`，兼容 V1
- **UI**：`UExQuestTreeWidget`
- **GameplayMessageRouter**：插件已声明依赖（P3 事件 → 任务进度）

---

## 概念与术语（中文读者必读）

### 一句话对照

| 英文类型 | 代码字段 | 中文理解 | 不是什么 |
|----------|----------|----------|----------|
| **Task** | `TaskId` | 一条任务线（主线 / 支线 / 一个 POI） | 不是 Actor 实例 |
| **Objective** | `ObjectiveTag` | 该任务下的可追踪条目（勾选、计数） | 不是子任务；不是实例 ID |
| **SubTask** | 另一个 `TaskId` | 挂在父任务下的独立子任务线 | 不是 Objective 的别名 |

### 层级结构

```
Task (TaskId = Quest.Main_001)     ← 「拯救世界」
  ├── Objectives[]
  │     ├── ObjectiveTag: ...Obj_001   ← 找圣剑
  │     └── ObjectiveTag: ...Obj_002   ← 打魔王
  └── SubTaskIds → Task (Quest.Main_001.Sub_001)   ← 支线「收集装备」
```

- **Objective**：写在父 Task 的 `Objectives[]` 里，无独立 Locked/Active。
- **SubTask**：`AllTasks` 中另一条 Task，有自己的 `State`。

### 为何示例里「找圣剑 / 打魔王」是 Objective？

| 内容 | 建模 | 原因 |
|------|------|------|
| 找圣剑、打魔王 | **Objective** | 同一条任务下的检查清单 |
| 收集装备 | **SubTask** | 独立状态、可单独解锁 |

也可把主线两步做成 **SubTask**（需 `PreTaskIds`）。系统两种都支持。

| 需求 | 更合适 |
|------|--------|
| 同标题下多勾选 / 计数 | **Objective** |
| 单独解锁、单独一条任务、地图 POI 级子任务 | **SubTask** |
| POI 内开机 / 击杀 / 撤离 | 该 POI **Task** 下的 **Objective** |

### 常见误解

1. **Objective ≠ 子任务**
2. **ObjectiveTag ≠ 场景 Actor**
3. 中文「任务」可能指 Task 或 Objective，以 `TaskId` / `ObjectiveTag` 为准
4. 改进度：`IncrementQuestObjective(TaskId, ObjectiveTag, Delta)`；改任务状态用 `TaskId`
5. 蓝图：**Quest Task** = Latent；**Make Quest Task Data** = 拼结构体

### 全局任务 + 多 POI 子任务

| 设计概念 | 建模 |
|----------|------|
| 全局 / 战役级任务 | 一个 **TaskId** |
| 每个 POI 点位 | **SubTask**（`ParentTaskId` 指向全局） |
| POI 内具体步骤 | 该 POI **Task** 下的 **ObjectiveTag** |

### Tag 命名

```
TaskId:         Quest.Op.Global.Op01
TaskId:         Quest.POI.Factory_A
ObjectiveTag:   Quest.POI.Factory_A.PowerOn
```

同一 QuestSet 内 **ObjectiveTag 建议全局唯一**（供 `NotifyObjectiveProgressByTag` 反查 Task）。

---

## 快速开始

### 创建数据

```cpp
FExQuestData QuestData = UExQuestBlueprintLibrary::CreateExampleQuestData();
```

或蓝图 **Create Example Quest Data**；拼装单条任务用 **Make Quest Task Data**（不是 Quest Task 节点）。

### 加载

```cpp
QuestManager->LoadQuestData(QuestData);
// 或
QuestManager->LoadQuestFromAsset(MyQuestAsset, false);
```

### 更新

```cpp
QuestManager->ActivateQuest(TaskId);
QuestManager->IncrementQuestObjective(TaskId, ObjectiveTag, 1);
```

---

## 数据结构摘要

- **FExQuestObjective**：`ObjectiveTag`、进度、`bIsOptional`
- **FExQuestTask**：`TaskId`、`State`、`Objectives`、`SubTaskIds`、`PreTaskIds`、`ParentTaskId`
- **FExQuestData**：`AllTasks` 扁平列表 + `RebuildIndices()`

---

## 任务状态

`Inactive` → `Active` → `Completed` / `Failed`；`Locked` 需解锁或前置完成。

---

## Latent 集成

- 通用：**Create Latent Task** + 手动调 Quest API
- 推荐：**Quest Task** 节点 + `UExLatentTask_QuestBound`（`BoundQuestTag` = TaskId，`BoundObjectiveTag`）

---

## 相关文档

- [QuestDevPlan.md](./QuestDevPlan.md) — 开发阶段
- [README.md](./README.md) — **文档 UTF-8 约定**
- [Usage.md](./Usage.md) / [Architecture.md](./Architecture.md) — 插件节点与架构
'''

Path(__file__).resolve().parents[1].joinpath("Docs", "QuestSystemGuide.md").write_text(
    GUIDE, encoding="utf-8", newline="\n"
)
print("QuestSystemGuide.md written (UTF-8)")
