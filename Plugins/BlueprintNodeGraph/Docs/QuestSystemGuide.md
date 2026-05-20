# 任务系统指南

> UTF-8。通用 Latent 节点见 [Usage.md](./Usage.md)；类与数据流见 [Architecture.md](./Architecture.md#任务系统)。

---

## 概述

层级任务系统：`UExQuestDataAsset`（策划）→ Server Load → `UExQuestManagerSubsystem` → 联机复制 → `UExQuestTreeWidget`。

| 能力 | 说明 |
|------|------|
| 数据 | `DT_Quest_*` → `DA_Quest_*`（`FExQuestTaskTableRow`） |
| 玩法链 | `AExQuestAgentActor` + **Quest Task** Latent |
| 联机 | `UExQuestReplicationComponent` on GameState，蓝图库 **Route** API |
| 解耦推进 | GameplayMessageRouter → `NotifyObjectiveProgressByTag` |
| 存档 | JSON `#ExQuestSaveV2` |

---

## 术语

| 类型 | 字段 | 含义 |
|------|------|------|
| **Task** | `TaskId` | 一条任务线（主线 / POI / 支线），有独立 `State` |
| **Objective** | `ObjectiveTag` | Task 内勾选/计数项，**不是**子任务 |
| **SubTask** | 另一个 `TaskId` | 挂在父 Task 下，独立状态 |

```
Task (Quest.Main)
  ├── Objectives[]     ← 同 Task 内清单
  └── SubTaskIds[]     ← 指向 AllTasks 中另一行 Task
```

**建模**：同标题多勾选 → **Objective**；可单独解锁的一条线 / POI → **SubTask**。同一 QuestSet 内 **ObjectiveTag 建议全局唯一**。

**层级字段**（DT/DA 每行一条 Task）：

| 字段 | 用途 |
|------|------|
| `Parent Task Id`（子行必填） | 任务树 UI 挂父节点；**仅填父行 SubTaskIds 不会显示子任务** |
| `Sub Task Ids`（父行） | 父 Task 自动 Completed 的汇总（全部子 Task Completed） |
| `Objectives[]` | 该 Task 内进度清单 |

父 Task 自动 Completed（`IsReadyToComplete`）：**全部必填 Objective 完成** 且 **`SubTaskIds` 子 Task 均为 Completed**。

---

## 任务状态

| 状态 | API |
|------|-----|
| Locked → Inactive | `Unlock Quest` |
| Inactive → Active | `Activate Quest` / `Ensure Quest Active` |
| Active 中改 Objective | Quest Task / `Increment Quest Objective` |
| 结束 | `CompleteQuest` 或自动汇总 |

`Unlock` 不会直接变 Active；Quest Task 默认 OnStart **Auto Ensure Active**（Unlock + Activate）。

---

## 快速上手（BNGDemo）

插件路径 `/BlueprintNodeGraph/Quest/`：

| 资产 | 说明 |
|------|------|
| `DT_Quest_Test` | 策划表 |
| `DA_Quest_Test` | 运行时 Load（保存 DT 可自动同步） |
| `BP_QuestHost` | `AExQuestAgentActor`，每关 **一个** |
| `WBP_QuestTree` | `UExQuestTreeWidget`，`bAutoSyncFromManager = true` |

已在 Standalone / Listen Server / Dedicated Server + Client 验证。

### 数据

1. DataTable Row Type = `FExQuestTaskTableRow`，每行一条 Task。
2. 保存 `DT_Quest_*` → 默认同步 `DA_Quest_*`（`DefaultBlueprintNodeGraph.ini` → `bAutoImportQuestTableOnSave`）；或右键 **Import To Paired Quest Data Asset**。
3. Tag 注册于 `DefaultGameplayTags.ini`；运行时 **只 Load DA**。

示例行：

| TaskId | ParentTaskId | SubTaskIds |
|--------|--------------|------------|
| `Quest.Test.Main` | — | `Quest.Test.Sub_A` |
| `Quest.Test.Sub_A` | `Quest.Test.Main` | — |

### Agent 接线

**勿**在 BeginPlay 用 `Switch Has Authority` 混跑 Server 链与 UI：

```
Event On Server Ready     → Quest Task 链（仅 Authority，Auto Load DA 后）
Event On Client Ready     → Create WBP_QuestTree → Add to Viewport
```

| 模式 | Server Ready | Client Ready |
|------|:------------:|:------------:|
| Standalone / Listen 主机 | ✅ | ✅ |
| Dedicated Server | ✅ | ❌ |
| Dedicated Client | ❌ | ✅ |

- Client **不** Load DA、**不**跑 Quest Task 写回链。
- 薄触发（Terminal/Volume）只 **Call Custom Event on Agent**。
- 本图顺序在 Agent Quest Task 链；跨图硬门槛用 DA `PreTaskIds`。

### Quest Task 写回

继承 **`UExLatentTask_Quest`**，用 **Quest Task** 节点（勿 Create Latent Task 创建 Quest 子类）。

| Objective Tag | 成功结束 |
|---------------|----------|
| 有值 | Increment / Complete Objective |
| **None** | **CompleteQuest(Quest Tag)**（整 Task，适合 SubTask 整段玩法） |

高级引脚（节点 ▼）：Complete Action、Progress Delta、Apply Quest on Successful Stop、Auto Ensure Active On Start。

示意链：

```
Event On Server Ready
  → Quest Task (Main, Step1)      // Objective
  → Quest Task (Sub_A, None)      // Complete SubTask
```

### 联机

- 推荐 GameMode **Game State** = `AExQuestGameStateBase`。
- 兜底：Server 首次 Load DA 时 **`EnsureOnGameState`** 自动 Add Rep 组件。
- 玩法写回统一用蓝图库 **Route** 节点（Client → Server RPC）。

### 进测检查

- [ ] 关卡一个 `BP_QuestHost` + `DA_Quest_Test`
- [ ] 子 Task 独立行且 `Parent Task Id` 正确
- [ ] Server 链 / UI 分别在 Server Ready、Client Ready
- [ ] Client 不 Load DA；Dedicated Server 不 Create Widget

---

## 蓝图库（Route）

| 节点 | 用途 |
|------|------|
| Load Quest From Asset | Server/Standalone 加载 DA |
| Unlock / Activate / Ensure Quest Active | 状态 |
| Increment Quest Objective | Objective +Delta |
| Notify Objective Progress By Tag | 按 Tag 反查 Task |
| Ensure Quest Replication On GameState | 手动确保 Rep 组件 |
| Broadcast Quest Objective Progress | 发 MessageRouter 消息 |
| Make Quest Task Data | 拼结构体（**不是** Quest Task 节点） |
| Build Quest Data From Task Table | 调试直读表 |

---

## GameplayMessageRouter

```cpp
QuestManager->NotifyObjectiveProgressByTag(ObjectiveTag, 1);
// 或 Broadcast Quest Objective Progress → 频道 Quest.Event.Objective.Progress
```

`UExQuestMessageRouterBridge` 监听并 Route 到 Server Manager。仅 **Active** Task 累计进度。

---

## 分工

| 内容 | 位置 |
|------|------|
| 本图步骤顺序 | Agent Quest Task 链 |
| 任务树 / Objective / 汇总 | DA |
| 任务 UI | `WBP_QuestTree`（Client Ready） |
| 运行时权威 | Server Manager + GameState Rep |

---

## 相关文档

- [Usage.md](./Usage.md) — Loop Delay、Create Latent Task 等通用节点
- [Architecture.md](./Architecture.md) — 模块结构与运行时数据流
- [README.md](./README.md) — 文档 UTF-8 约定
