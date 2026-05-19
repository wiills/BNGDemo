# 任务系统使用指南

> 编码：UTF-8。术语说明见下文「概念与术语」。

## 概述

BlueprintNodeGraph 插件提供层级任务系统，支持：

- **扁平存储 + 父子关系**：`AllTasks` + `ParentTaskId` / `SubTaskIds`
- **GameplayTag**：`TaskId`（任务）与 `ObjectiveTag`（目标）
- **DataAsset**：`UExQuestDataAsset`（策划主入口，一张表含全局 + POI）
- **DataTable（可选）**：行结构 `FExQuestTaskTableRow`，与 `FExQuestTaskDefinition` 字段一致，便于 CSV / 表驱动
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

### DataTable（表驱动，与 DA 二选一或导入用）

1. 新建 DataTable，**Row Type** = `FExQuestTaskTableRow`（每行一条 Task，字段同 DA 里的 Task 行）。
2. 蓝图：**Build Quest Data From Task Table** → 得到 `FExQuestData`。
3. **Load Quest Data** 或 **Load Quest From Asset**（若仍用 DA 作主入口，可只在编辑器把表烘进 DA）。

行内 `Objectives` 为数组列；`SubTaskIds` / `PreTaskIds` 填 GameplayTag。运行时入口仍建议 **一次 Load** 一个 QuestSet。

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

### 子 Task → 父 Task 汇总

- 子线完成后，若父 Task 为 **Active** 且 `IsReadyToComplete`（本 Task 必填 Objective 已完成 + `SubTaskIds` 中全部子 Task 为 `Completed`），父 Task 自动标为 **Completed**，并递归检查更上层父 Task。
- 仅 **Objectives** 完成但子 Task 未齐时，父 Task **不会**自动完成。
- UI 进度：**Get Quest Aggregate Completion Percent With Data**（含子 Task）；仅 Objective 时用 **Get Quest Completion Percent**。
- 手动 `CompleteQuest(父)` 仍可强制完成（不校验子 Task），用于调试或剧情跳过。

---
- **FExQuestData**：`AllTasks` 扁平列表 + `RebuildIndices()`

---

## 任务状态

`Inactive` → `Active` → `Completed` / `Failed`；`Locked` 需解锁或前置完成。

---

## Latent 集成

- **基类**：所有 Quest 相关 Latent 蓝图继承 **`UExLatentTask_Quest`**（勿用 `UExLatentTask_Custom`）。
- **Quest Task** 节点 → `CreateQuestProxy`；`TryStop` 成功且 `bApplyQuestOnSuccessfulStop` 时写回 Objective（可 Blueprint 重写 `ApplyQuestOnComplete`）。
- 字段：`BoundQuestTag`（TaskId）、`BoundObjectiveTag`；`CompleteAction` 选增量或一次完成。
- 仅特殊流程：**Create Latent Task** + 手动 Quest API。

---

## P3：GameplayMessageRouter 与按 Tag 推进

### 直接调用

```cpp
QuestManager->NotifyObjectiveProgressByTag(ObjectiveTag, 1);
// 或蓝图：Notify Objective Progress By Tag
```

- 通过 `FindTaskIdByObjectiveTag` 解析 `TaskId`
- 仅当任务为 **Active** 时累计进度
- `Delta` 必须 > 0

### 发消息（解耦玩法）

```cpp
FExQuestObjectiveProgressMessage Msg;
Msg.ObjectiveTag = ...;
Msg.Delta = 1;
// 可选 Msg.TaskId 跳过反查
UGameplayMessageSubsystem::Get(World).BroadcastMessage(
    ExQuestMessageTags::GetObjectiveProgressChannel(), Msg);
```

蓝图：**Broadcast Quest Objective Progress**（`UExQuestBlueprintLibrary`）。

`UExQuestMessageRouterBridge`（GameInstance 子系统）监听频道 `Quest.Event.Objective.Progress` 并写入 Manager。

### Tag 配置

- 插件 `Config/DefaultGameplayTags.ini` 含示例 Tag（与 `CreateExampleQuestData` 对齐）
- 频道 Tag 亦通过 `NativeGameplayTags` 注册

---

## 联机：全队共享进度（Standalone / Server / Client）

### 原则

- **Server / Standalone** 为权威端；**Client** 只通过 `UExQuestReplicationComponent` 发 Server RPC，不直接改本地 Manager 进度。
- 全队共享同一份运行时状态：`FExQuestRuntimeState` + `UExQuestDataAsset` 挂在 **GameState** 上复制。
- 无 Replication 组件时，蓝图库 **Route** API 会回退到本地 `UExQuestManagerSubsystem`（兼容纯单机或未挂组件的项目）。

### 接入步骤

1. 将 GameState 设为 `AExQuestGameStateBase`，或在自有 GameState 上添加 **`UExQuestReplicationComponent`**。
2. 开局在 **Server / Standalone** 调用 **Load Quest From Asset**（会复制到所有客户端）。
3. 玩法侧统一使用蓝图库节点（内部已走 Route）：
   - `Unlock Quest` / `Increment Quest Objective` / `Notify Objective Progress By Tag`
   - `Load Quest Progress From Json` / `Apply Runtime State To Manager`
4. `UExQuestMessageRouterBridge` 在收到 `Quest.Event.Objective.Progress` 时同样走 Route（Client 广播不会进 Server Manager）。

### 数据流（简图）

```
Client 蓝图/消息 → Route* → Server RPC → Manager（权威）
                              ↓
                    PublishState → GameState Rep
                              ↓
Client OnRep → ApplyReplicatedQuestView → 本地 Manager + UI 刷新
```

### 注意

- **Listen Server**：主机为权威端，与 Dedicated 相同。
- **存档**：建议在 Server 上 `Save Quest Progress`，再按需 `Load Quest Progress From Json`（经 Route）。
- **配置**仍以 `UExQuestDataAsset` 为主；复制的是运行时进度，不是整表替换。

---

## 相关文档

- [QuestDevPlan.md](./QuestDevPlan.md) — 开发阶段
- [README.md](./README.md) — **文档 UTF-8 约定**
- [Usage.md](./Usage.md) / [Architecture.md](./Architecture.md) — 插件节点与架构
