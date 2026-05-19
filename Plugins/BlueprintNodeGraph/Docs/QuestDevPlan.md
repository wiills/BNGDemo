# Quest 系统开发计划

> 基于代码审查结论，分阶段落地。状态：**P0/P1 已完成（2026-05）**

## 阶段概览

| 阶段 | 目标 | 状态 |
|------|------|------|
| **P0** | 状态机正确性、完成后解锁链、存档/重置/事件 | ✅ 已完成 |
| **P1** | UI 可用性（展开、标题、委托）、Blueprint API 补全 | ✅ 已完成 |
| **P2** | Tag 索引、配置与运行时分离（DataAsset） | ✅ 已完成 |
| **P3** | GameplayTag 驱动目标、LatentTask 桥接 | 待办 |

---

## P0 — 核心逻辑（当前迭代）

### P0.1 状态语义

- [x] `CanActivate()` 仅允许 `Inactive`
- [x] `UnlockQuest`：`Locked` → `Inactive`（校验前置任务）
- [x] 文档明确：`Locked` 不可直接激活，需先解锁

### P0.2 完成后级联

- [x] 任务完成时扫描 `PreTaskIds` 含该 ID 的任务，前置满足则 `Locked` → `Inactive`
- [x] 父任务完成时，子任务（`ParentTaskId`）`Locked` → `Inactive`
- [x] `CompleteQuest` / 目标自动完成 共用 `HandleQuestCompleted`

### P0.3 目标进度

- [x] `UpdateQuestObjective` 进度低于目标时清除 `bIsCompleted`
- [x] `IncrementQuestObjective`（Delta 增量）

### P0.4 重复任务

- [x] `bIsRepeatable`：完成后重置为 `Inactive` 并清空目标进度

### P0.5 重置与存档

- [x] `LoadQuestData` 记录各任务 `InitialState`
- [x] `ResetAllQuests` 恢复初始状态并广播
- [x] 存档格式 `#ExQuestSaveV1` 版本头；兼容无版本旧档
- [x] `LoadQuestProgress` 失败返回 `false`；成功后广播刷新

### P0.6 委托

- [x] `OnQuestDataLoaded`（加载数据/进度后）

---

## P1 — UI 与蓝图（已完成）

- [x] `ExQuestTreeWidget`：展开/收起按钮接线
- [x] 更新 `TitleText`
- [x] 监听 `OnQuestObjectiveUpdated` / `OnQuestDataLoaded`
- [x] Widget 复用 `UExQuestBlueprintLibrary` 状态文本/颜色
- [ ] `WBP_QuestItem` 条目（可选，替换纯 C++ TextBlock）

---

## P2 — 规模与数据（已完成）

- [x] `FExQuestData::RebuildIndices()` — Task / Objective / Parent / PreTask 反向索引
- [x] `UExQuestDataAsset` + `FExQuestTaskDefinition` / `FExQuestObjectiveDefinition`
- [x] `FExQuestRuntimeState` + `ExtractRuntimeState` / `ApplyRuntimeState`
- [x] `LoadQuestFromAsset`（可选 `bPreserveRuntime`）
- [x] JSON 存档 `#ExQuestSaveV2`（`SaveQuestProgress` 默认 JSON；兼容 V1 文本加载）
- [x] `SaveQuestProgressAsTextV1` 保留旧格式导出

---

## P3 — 玩法集成（后续）

- [ ] `NotifyObjectiveByTag(ObjectiveTag, Delta)` 全局映射
- [ ] `UExLatentTask_Custom` 可选 `QuestTaskId` / `ObjectiveId` 属性
- [ ] DefaultGameplayTags.ini 示例 Tag 列表

---

## 文件变更清单（P0）

| 文件 | 变更 |
|------|------|
| `ExQuestTypes.h/.cpp` | `CanActivate`、查询依赖任务 |
| `ExQuestManagerSubsystem.h/.cpp` | 级联、解锁、存档 v1、初始状态 |
| `ExQuestBlueprintLibrary.h/.cpp` | `UnlockQuest`、`IncrementQuestObjective` |
| `ExQuestTreeWidget.cpp` | 委托、展开按钮、标题 |
| `QuestSystemGuide.md` | 状态说明与 API 更新 |

---

## 验收标准（P0）

1. 示例数据中 `Main_002` 在 `Main_001` 完成前无法激活；完成后自动变为可激活（`Inactive`）
2. `Locked` 任务调用 `ActivateQuest` 返回 `false`；`UnlockQuest` 或前置完成后可激活
3. `ResetAllQuests` 恢复 `LoadQuestData` 时的初始状态，UI 同步刷新
4. 存档带 `V1` 头可往返；旧格式无头仍可加载
