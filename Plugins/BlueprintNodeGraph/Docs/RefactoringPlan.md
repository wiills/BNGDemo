# BlueprintNodeGraph 重构计划

> **状态**：框架开发阶段，可自由改类名与目录，无需考虑蓝图资产兼容。  
> **代码重构**：✅ 已完成（2026-05-19）  
> **文档同步**：✅ 已完成（2026-05-19）

---

## 1. 概述

本次重构分两层收益：

| 层级 | 内容 | 状态 |
|------|------|------|
| **目录重组** | 按类型分子目录，解决文件难找 | ✅ 完成 |
| **类名统一** | `ExAsyncAction_*` / `ExLatentTask_*` / `ExProxy_*` / `ExBase_*` | ✅ 完成 |
| **文档同步** | README、Usage、Architecture 等引用更新 | ✅ 完成 |

自动化脚本：`Scripts/refactor_rename.py`（含完整文件映射与类名映射）

---

## 2. 当前进度

### 已完成 ✅ — Runtime 模块（BlueprintNodeGraph）

| 目录 | 文件 |
|------|------|
| **Common/** | ExLatentProxyDefine, ExWaitBranchCompletionMode, ExBlueprintDebugBubble, ExK2NodeTimeoutLatentAction, ExWaitAction, ExSubsystemGetter, ExSaveGameTypes |
| **AsyncActions/** | ExBase_AsyncAction, ExAsyncAction_LoadAsset, StreamLevel, SpawnActor, GameplayTag, Network |
| **Proxies/** | ExBase_FlowProxy, ExProxy_WaitCondition, WaitBranch, BlendPercent, LoopDelay, ForLoopWithDelay, LatentTask |
| **LatentTasks/** | ExBase_LatentTask, ExLatentTaskInterface, ForAttach, Saveable |
| **Subsystems/** | ExLatentActionManager, ExBlueprintNodeGraphDebugSubsystem, ExWorldPartitionSubsystem |
| **Libraries/** | ExBlueprintNodeLibrary, ExSaveGameLibrary |
| **Assets/** | ExGraphAsset, ExLevelTrigger |

### 已完成 ✅ — Editor 模块（BlueprintNodeGraphEditor）

| 目录 | 文件 |
|------|------|
| **K2Nodes/** | ExK2Node_AsyncBase, ShowBase, SwitchValue, WaitCondition, WaitAll, WaitAny, WaitCount, LoadAsset, StreamLevel, LoopDelay, ForLoopWithDelay, AsyncBlendPercent, GameplayTag, LatentTaskObject, LatentTaskCall, CreateTaskAsync |
| **Slate/** | SGraphNode_ShowBase, SGraphNode_ExAsyncDebug |
| **AssetActions/** | ExAssetTypeActions, ExAssetTypeActions_FlowGraph |

### 文档同步 ✅

| 文档 | 状态 |
|------|------|
| `README.md` / `README_CN.md` | ✅ 已更新路径 |
| `Docs/Architecture.md` | ✅ 已更新继承关系与目录结构 |
| `Docs/Usage.md` | ✅ 已更新 ExProxy_LatentTask 示例 |
| `Docs/QuestSystemGuide.md` | ✅ 已使用新类名 |

---

## 3. 核心类型体系

> 实际代码中存在 **四类** 运行时对象。归类必须依据 **继承关系**，不能只看文件名里的 `Proxy` / `Async` 字样。

### 继承关系总览

```
UObject
│
├── UExBase_AsyncAction                    # 一次性异步操作基类
│   ├── UExAsyncActionProxy                # 带 Success/Failure 委托
│   ├── UExAsyncAction_BranchSync          # 多分支计数等待（原 UExLatentActionProxy）
│   ├── UExAsyncAction_LoadAsset
│   ├── UExAsyncAction_StreamLevel
│   ├── UExAsyncAction_SpawnActor          # 原 ExLatentSpawnProxy
│   ├── UExAsyncAction_GameplayTag*        # Listener / Query / Modifier
│   └── UExAsyncAction_Replication/SaveGame/Checkpoint
│
├── UExBase_FlowProxy                      # 流程控制代理基类（原 UExLatentActionProxyBase）
│   ├── UExProxy_WaitCondition
│   ├── UExProxy_WaitBranch
│   ├── UExProxy_BlendPercent              # 原 ExAsyncBlendPercentProxy
│   ├── UExProxy_LoopDelay
│   └── UExProxy_ForLoopWithDelay
│
└── UExBase_LatentTask                     # 可 Blueprint 子类化的延迟任务
    ├── IExLatentTaskInterface
    ├── UExLatentTask_ForAttach
    ├── UExLatentTask_Saveable
    ├── UExProxy_LatentTask                # K2 工厂（原 ExLatentTaskProxy）
    └── UExProxy_LatentTaskUUID            # 带 UUID 同步的工厂
```

### 类型对照表

| 类型 | 前缀 | 基类 | 特征 | 典型场景 |
|------|------|------|------|----------|
| **AsyncAction** | `ExAsyncAction_` | `UExBase_AsyncAction` | 一次性异步，Activate → 完成 | 加载资源、流关卡、Spawn、网络同步 |
| **FlowProxy** | `ExProxy_` | `UExBase_FlowProxy` | K2 节点驱动，多输入分支、Tick 轮询 | WaitCondition、LoopDelay、BlendPercent |
| **LatentTask** | `ExLatentTask_` | `UExBase_LatentTask` | 可持续运行，支持超时/暂停，可 BP 继承 | 自定义任务逻辑、Quest Task |
| **LatentTask 工厂** | `ExProxy_LatentTask` | `UExBase_LatentTask` | K2 用来 **创建** LatentTask 实例 | `CreateLatentTask` 节点 |

### 易混淆点（必读）

| 旧名 | 常见误解 | 实际归属 |
|------|----------|----------|
| `ExLatentSpawnProxy` | LatentTask | **AsyncAction**（继承 `UExAsyncActionBase`） |
| `ExAsyncBlendPercentProxy` | AsyncAction | **FlowProxy**（继承 `UExLatentActionProxyBase`） |
| `ExGameplayTagProxy` | FlowProxy | **AsyncAction**（继承 `UExAsyncActionBase`） |
| `ExLatentTaskProxy` | LatentTask 基类 | **工厂 Proxy**（K2 创建入口） |
| `UExLatentActionProxy` | FlowProxy | **AsyncAction 分支**（继承 `UExBase_AsyncAction`） |

---

## 4. 命名规范

### 前缀规则

| 类别 | 文件前缀 | 类名模式 | 示例 |
|------|----------|----------|------|
| K2 节点 | `ExK2Node_` | `ExK2Node_` + 功能 | `ExK2Node_WaitCondition` |
| 异步操作 | `ExAsyncAction_` | `UExAsyncAction_` + 功能 | `UExAsyncAction_LoadAsset` |
| 流程代理 | `ExProxy_` | `UExProxy_` + 功能 | `UExProxy_WaitCondition` |
| 延迟任务 | `ExLatentTask_` | `UExLatentTask_` + 功能 | `UExLatentTask_ForAttach` |
| 抽象基类 | `ExBase_` | `UExBase_` + 类型 | `UExBase_AsyncAction` |
| 子系统 | `Ex` + 名称 + `Subsystem/Manager` | 保持语义 | `UExLatentActionManager` |
| 函数库 | `Ex` + 名称 + `Library` | 保持语义 | `UExBlueprintNodeLibrary` |

### Include 路径约定

统一使用完整模块路径，**无需**修改 Build.cs：

```cpp
#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"
#include "BlueprintTool/Proxies/ExProxy_WaitCondition.h"
#include "BlueprintTool/LatentTasks/ExBase_LatentTask.h"
```

---

## 5. 目标目录结构（当前实际结构）

### Runtime 模块

```
BlueprintTool/
├── AsyncActions/
│   ├── ExBase_AsyncAction.h              # UExBase_AsyncAction + UExAsyncActionProxy
│   ├── ExAsyncAction_LoadAsset.h
│   ├── ExAsyncAction_StreamLevel.h
│   ├── ExAsyncAction_SpawnActor.h
│   ├── ExAsyncAction_GameplayTag.h       # Listener / Query / Modifier
│   └── ExAsyncAction_Network.h           # Replication / SaveGame / Checkpoint
│
├── Proxies/
│   ├── ExBase_FlowProxy.h                # UExBase_FlowProxy + UExAsyncAction_BranchSync
│   ├── ExProxy_WaitCondition.h
│   ├── ExProxy_WaitBranch.h
│   ├── ExProxy_BlendPercent.h
│   ├── ExProxy_LoopDelay.h
│   ├── ExProxy_ForLoopWithDelay.h
│   └── ExProxy_LatentTask.h              # UExProxy_LatentTask + UExProxy_LatentTaskUUID
│
├── LatentTasks/
│   ├── ExBase_LatentTask.h
│   ├── ExLatentTaskInterface.h
│   ├── ExLatentTask_ForAttach.h
│   └── ExLatentTask_Saveable.h
│
├── Subsystems/
│   ├── ExLatentActionManager.h           # 仅 Manager + CreateWaitProxyCall 模板
│   ├── ExBlueprintNodeGraphDebugSubsystem.h
│   └── ExWorldPartitionSubsystem.h
│
├── Libraries/
│   ├── ExBlueprintNodeLibrary.h
│   └── ExSaveGameLibrary.h
│
├── Common/
│   ├── ExLatentProxyDefine.h
│   ├── ExWaitBranchCompletionMode.h
│   ├── ExSaveGameTypes.h
│   ├── ExBlueprintDebugBubble.h
│   ├── ExK2NodeTimeoutLatentAction.h
│   ├── ExWaitAction.h
│   └── ExSubsystemGetter.h
│
└── Assets/
    ├── ExGraphAsset.h
    └── ExLevelTrigger.h
```

### Editor 模块

```
BlueprintTool/
├── K2Nodes/          # 16 个 K2 节点（类名保持不变）
├── Slate/            # SGraphNode_ShowBase, SGraphNode_ExAsyncDebug
└── AssetActions/     # ExAssetTypeActions, ExAssetTypeActions_FlowGraph
```

---

## 6. 完整改名清单（已执行）

> 状态列均为 ✅。原 `ExLatentActionManager.h` 已拆分为 `Proxies/ExBase_FlowProxy.h` + `Subsystems/ExLatentActionManager.h`。

### 6.1 AsyncAction ✅

| 原文件 | 新文件 | 原类名 | 新类名 |
|--------|--------|--------|--------|
| ExAsyncActionBase.h | AsyncActions/ExBase_AsyncAction.h | UExAsyncActionBase | UExBase_AsyncAction |
| ExAsyncLoadAsset.h | AsyncActions/ExAsyncAction_LoadAsset.h | UExAsyncLoadAsset | UExAsyncAction_LoadAsset |
| ExAsyncStreamLevel.h | AsyncActions/ExAsyncAction_StreamLevel.h | UExAsyncStreamLevel | UExAsyncAction_StreamLevel |
| ExLatentSpawnProxy.h | AsyncActions/ExAsyncAction_SpawnActor.h | UExLatentSpawnProxy | UExAsyncAction_SpawnActor |
| ExGameplayTagProxy.h | AsyncActions/ExAsyncAction_GameplayTag.h | UExGameplayTagListenerProxy 等 | UExAsyncAction_GameplayTag* |
| ExNetworkProxy.h | AsyncActions/ExAsyncAction_Network.h | UExReplicationProxy 等 | UExAsyncAction_Replication/SaveGame/Checkpoint |
| — | Proxies/ExBase_FlowProxy.h | UExLatentActionProxy | UExAsyncAction_BranchSync |

### 6.2 FlowProxy ✅

| 原文件 | 新文件 | 原类名 | 新类名 |
|--------|--------|--------|--------|
| ExLatentActionManager.h（部分） | Proxies/ExBase_FlowProxy.h | UExLatentActionProxyBase | UExBase_FlowProxy |
| ExWaitConditionProxy.h | Proxies/ExProxy_WaitCondition.h | UExWaitConditionProxy | UExProxy_WaitCondition |
| ExWaitBranchProxy.h | Proxies/ExProxy_WaitBranch.h | UExWaitBranchProxy | UExProxy_WaitBranch |
| ExAsyncBlendPercent.h | Proxies/ExProxy_BlendPercent.h | UExAsyncBlendPercentProxy | UExProxy_BlendPercent |
| ExLoopDelayProxy.h | Proxies/ExProxy_LoopDelay.h | UExLoopDelayProxy | UExProxy_LoopDelay |
| ExForLoopWithDelayProxy.h | Proxies/ExProxy_ForLoopWithDelay.h | UExForLoopWithDelayProxy | UExProxy_ForLoopWithDelay |

### 6.3 LatentTask ✅

| 原文件 | 新文件 | 原类名 | 新类名 |
|--------|--------|--------|--------|
| ExLatentTaskBase.h | LatentTasks/ExBase_LatentTask.h | UExLatentTaskBase | UExBase_LatentTask |
| ExLatentTaskForAttach.h | LatentTasks/ExLatentTask_ForAttach.h | UExLatentTaskForAttach | UExLatentTask_ForAttach |
| ExSaveableLatentTask.h | LatentTasks/ExLatentTask_Saveable.h | UExSaveableLatentTask | UExLatentTask_Saveable |

### 6.4 LatentTask 工厂（归入 Proxies/）✅

| 原文件 | 新文件 | 原类名 | 新类名 |
|--------|--------|--------|--------|
| ExLatentTaskProxy.h | Proxies/ExProxy_LatentTask.h | UExLatentTaskProxy | UExProxy_LatentTask |
| ExLatentTaskProxy.h | Proxies/ExProxy_LatentTask.h | UExLatentTaskUUIDProxy | UExProxy_LatentTaskUUID |

### 6.5 Subsystems / Common / Libraries / Assets ✅

| 原文件 | 新目录 | 类名 |
|--------|--------|------|
| ExLatentActionManager.h（Manager 部分） | Subsystems/ | UExLatentActionManager 不变 |
| ExBlueprintNodeGraphDebugSubsystem.h | Subsystems/ | 不变 |
| ExWorldPartitionSubsystem.h | Subsystems/ | 不变 |
| ExLatentProxyDefine.h 等 7 个 | Common/ | 不变 |
| ExBlueprintNodeLibrary.h | Libraries/ | 不变 |
| ExSaveGameLibrary.h | Libraries/ | 不变 |
| ExGraphAsset.h / ExLevelTrigger.h | Assets/ | 不变 |

### 6.6 Editor ✅

| 原位置 | 新目录 | 类名变更 |
|--------|--------|----------|
| BlueprintTool/ExK2Node_*.h | K2Nodes/ | 保持不变 |
| BlueprintTool/SGraphNode_*.h | Slate/ | 保持不变 |
| BlueprintTool/ExAssetTypeActions*.h | AssetActions/ | 保持不变 |

---

## 7. 修改要点（供参考）

### Include 路径

```cpp
// 修改前
#include "BlueprintTool/ExAsyncActionBase.h"

// 修改后
#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"
```

### 类名引用

```cpp
// 修改前
UExAsyncActionBase* AsyncProxy = Cast<UExAsyncActionBase>(ProxyObject);

// 修改后
UExBase_AsyncAction* AsyncProxy = Cast<UExBase_AsyncAction>(ProxyObject);
```

### Build.cs

**无需修改**。UE 模块 `Public/` 子目录默认可 include；使用完整路径即可。

---

## 8. 执行记录

重构通过 `Scripts/refactor_rename.py` 一次性完成：

1. 拆分 `ExLatentActionManager.h` → `ExBase_FlowProxy` + `ExLatentActionManager`
2. 迁移 Runtime / Editor 全部源文件到子目录
3. 批量更新类名与 `#include` 引用
4. 全局扫描修复残留引用

若需重新执行或查阅映射，见脚本内 `FILE_MAP_*` 与 `CLASS_RENAMES`。

---

## 9. 注意事项

1. **框架阶段可大胆改**：当前无已发布蓝图资产依赖，无需 CoreRedirects。
2. **归类看继承，不看文件名**：文件名含 `Proxy` 或 `Async` 不代表真实类型。
3. **工厂类单独命名**：`ExLatentTaskProxy` → `ExProxy_LatentTask`，避免与 `ExBase_LatentTask` 混淆。
4. **废弃类**：`UExProxy_LoopDelay` 仍保留 `UE_DEPRECATED` 标记，后续可评估移除。
5. **编译验证**：重构后需全量编译，让 UHT 重新生成 `.generated.h`。

---

## 附录：Quick Reference

```
看到类名前缀          →  它是什么
─────────────────────────────────────
UExBase_AsyncAction   →  异步操作根
UExAsyncAction_*      →  具体异步操作
UExBase_FlowProxy     →  流程代理根（K2 多分支/Tick）
UExProxy_*            →  具体流程代理 或 LatentTask 工厂
UExBase_LatentTask    →  可 BP 继承的延迟任务根
UExLatentTask_*       →  具体延迟任务
ExK2Node_*            →  编辑器蓝图节点（Editor 模块）
```
