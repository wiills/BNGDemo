# Quest 开发计划

> 文档编码 UTF-8。阶段以 P0/P1/P2/P3 划分（2026-05）。

## 总览

| 阶段 | 内容 | 状态 |
|------|------|------|
| **P0** | 状态机、解锁链、目标进度 | 已完成 |
| **P1** | UI、Blueprint API | 已完成 |
| **P2** | Tag 索引、DataAsset、JSON 存档 | 已完成 |
| **P3** | GameplayTag 反查、MessageRouter、Latent 绑定 | 已完成 |

---

## P3 已完成

- [x] `ObjectiveTag` 全局反查（同 QuestSet 内建议唯一）
- [x] `UExLatentTask_Quest`（Quest Latent 基类）+ `UExK2Node_QuestTask`
- [x] `ExK2Node_LatentTaskObject` 禁止选择 Quest 子类
- [x] 接入 **GameplayMessageRouter**
- [x] 区分 **Make Quest Task Data** 与 **Quest Task** 节点
- [x] `NotifyObjectiveProgressByTag(ObjectiveTag, Delta)`
- [x] `UExQuestMessageRouterBridge` 监听 `Quest.Event.Objective.Progress`
- [x] `Config/DefaultGameplayTags.ini` 示例 Tag
- [x] `BroadcastQuestObjectiveProgress`（发消息入口）

### P3.1 联机（全队共享）

- [x] `UExQuestReplicationComponent`（GameState，Server 权威 + Rep）
- [x] `AExQuestGameStateBase` 默认挂载复制组件
- [x] 蓝图库 / MessageRouter / Quest Latent 走 **Route** API
- [x] `Standalone` / `Listen Server` / `Dedicated` / `Client` 行为一致（Client → Server RPC）

### P3.1 可选（未做）

- [x] **子 Task 完成汇总到父 Task**（`TryRollUpParentTasks` / `IsReadyToComplete`）
- [ ] `Quest.Event.Objective.Complete` / `Quest.Event.Task.Unlock` 等频道
- [ ] 示例 `UExQuestDataAsset`（全局 + 多 POI）
- [x] `FExQuestTaskTableRow` + `BuildQuestDataFromTaskTable`（DataTable 行结构与 DA 任务行一致）
- [x] DataTable → DA 编辑器导入（`DT_Quest_*` → `DA_Quest_*`，内容浏览器右键）

### 子 Task 汇总（设计说明）

层级关系是 **Task → SubTask（另一条 Task）** 与 **Task → Objectives[]** 并列，不是 SubTask → Objective。

| 关系 | 含义 |
|------|------|
| `ParentTaskId` / `SubTaskIds` | 子 **Task** 挂在父 **Task** 下，各有独立 `State` |
| `Objectives[]` | 父 Task 内的检查清单，用 `ObjectiveTag` 推进 |

可选功能应实现例如：**全部子 Task `Completed` 后，自动将父 Task 标为完成**（或按子 Task 数量驱动父 Task 进度）。  
若需要「父 Task 上一条总进度条」，应在父 Task 上配 **Objective** 或 UI 聚合展示，而不是把 SubTask 写进父 Objective。

子 Task 全部 `Completed` 且父 Task 自身 Objectives 已满足时，父 Task（`Active`）会自动 `Completed` 并继续向上汇总；进度百分比见 `GetAggregateCompletionPercent`。

---

## 文档

| 文件 | 说明 |
|------|------|
| [QuestSystemGuide.md](./QuestSystemGuide.md) | 术语、API、**联机接入** |
| [README.md](./README.md) | 文档 UTF-8 约定 |

---

## 验收（P0）

1. `Main_002` 在 `Main_001` 完成前为 `Locked`，完成后 `Inactive`
2. `Locked` 经 `UnlockQuest` 可进入 `Inactive`
3. `ResetAllQuests` 恢复初始状态
4. 兼容 V1/V2 存档格式
