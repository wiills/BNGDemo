# Quest 系统开发计划

> 分阶段落地任务系统。状态：**P0/P1/P2 已完成**；**P3 进行中**（2026-05）

## 阶段概览

| 阶段 | 目标 | 状态 |
|------|------|------|
| **P0** | 状态机、完成后解锁链、存档/重置/事件 | 已完成 |
| **P1** | UI、Blueprint API | 已完成 |
| **P2** | Tag 索引、DataAsset、JSON 存档 | 已完成 |
| **P3** | GameplayTag 驱动目标、MessageRouter、Latent 桥接 | 进行中 |

---

## P3 — 玩法集成（进行中）

- [x] `ObjectiveTag` 重命名（配置 Tag，非实例 ID）
- [x] `UExLatentTask_QuestBound` + `UExK2Node_QuestTask`
- [x] `ExK2Node_LatentTaskObject` 禁止选择 QuestBound 子类
- [x] 插件依赖 `GameplayMessageRouter`
- [x] 蓝图 **Make Quest Task Data** 与 **Quest Task** 节点区分
- [ ] `NotifyObjectiveProgressByTag(ObjectiveTag, Delta)`
- [ ] `UExQuestMessageRouterBridge` 监听 `Quest.Event.Objective.Progress`
- [ ] `DefaultGameplayTags.ini` 示例 Tag

---

## 文档

| 文件 | 说明 |
|------|------|
| [QuestSystemGuide.md](./QuestSystemGuide.md) | 使用指南、**Task/Objective/SubTask 术语（中文）** |
| [README.md](./README.md) | 文档 UTF-8 编写约定 |

---

## 验收参考（P0）

1. `Main_002` 在 `Main_001` 完成前不可激活；完成后自动 `Inactive`
2. `Locked` 需 `UnlockQuest` 或前置完成
3. `ResetAllQuests` 恢复初始状态
4. 存档 V1/V2 兼容加载
