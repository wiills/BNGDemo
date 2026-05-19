# BlueprintNodeGraph 架构指南

> 编码：UTF-8。说明插件**模块划分、类职责与运行时机制**；蓝图节点用法见 [Usage.md](./Usage.md)，Quest 业务见 [QuestSystemGuide.md](./QuestSystemGuide.md)。

---

## 总览

插件拆为两个模块，运行时负责逻辑与网络，编辑器负责 K2 节点编译与 Slate。

| 模块 | 职责 |
|------|------|
| **BlueprintNodeGraph** | Proxy、LatentTask、AsyncAction、Subsystem、Quest 运行时 |
| **BlueprintNodeGraphEditor** | ExK2Node_* 节点、ExpandNode 编译、资产菜单、调试 UI |

三条主线：

1. **Flow Proxy**（UExBase_FlowProxy）— 蓝图异步流程节点，多路 Exec 汇合、定时、条件等待。  
2. **Latent Task**（UExBase_LatentTask）— 可 Spawn、可复制的 UObject 任务，适合长流程与存档。  
3. **Async Action**（UExBase_AsyncAction）— 资源加载、关卡流、GameplayTag、网络存档等一次性异步。

---

## 核心类（运行时）

| 基类 | 典型子类 | 用途 |
|------|----------|------|
| UExBase_FlowProxy | UExProxy_LoopDelay、UExProxy_ForLoopWithDelay、UExProxy_WaitCondition、UExProxy_WaitBranch、UExProxy_BlendPercent | 由 K2 节点创建，经 UExLatentActionManager 按 UUID 复用/缓存 |
| UExBase_LatentTask | UExLatentTask_Custom（蓝图父类）、UExLatentTask_Saveable、UExLatentTask_QuestBound、UExLatentTask_BranchSync（K2 内部） | TryStart / TryStop / Terminate，RunningState 可复制 |
| UExBase_AsyncAction | UExAsyncAction_LoadAsset、UExAsyncAction_StreamLevel、UExAsyncAction_GameplayTag*、UExAsyncAction_*（网络/存档） | 委托驱动，常配合 RegisterWithGameInstance |

**蓝图作者入口**：任务蓝图继承 UExLatentTask_Custom 或 UExLatentTask_Saveable；Quest 绑定用 UExLatentTask_QuestBound + UExK2Node_QuestTask。

---

## 目录结构（Source/）

| 路径 | 内容 |
|------|------|
| BlueprintNodeGraph/Public/BlueprintTool/Proxies/ | 流程代理实现 |
| BlueprintNodeGraph/.../LatentTasks/ | 延迟任务基类与用户扩展点 |
| BlueprintNodeGraph/.../AsyncActions/ | 异步 Action |
| BlueprintNodeGraph/.../Subsystems/ | UExLatentActionManager、调试、WorldPartition 等 |
| BlueprintNodeGraph/.../Common/ | FExLatentNodeInfo、分支模式、超时 Action |
| BlueprintNodeGraph/Public/Quest/ | UExQuestManagerSubsystem、UExQuestDataAsset、UI、BlueprintLibrary |
| BlueprintNodeGraphEditor/.../K2Nodes/ | 全部 UExK2Node_* |
| BlueprintNodeGraphEditor/.../Slate/、AssetActions/ | 编辑器 UI 与资产操作 |

---

## K2 节点 ↔ 运行时（对照）

| K2 节点（编辑器） | 运行时对象 |
|-------------------|------------|
| DelayInLoop | UExProxy_LoopDelay |
| For Loop With Delay | UExProxy_ForLoopWithDelay |
| Wait Condition | UExProxy_WaitCondition |
| Wait All / Wait Any / Wait Count | UExProxy_WaitBranch |
| AsyncBlendPercent | UExProxy_BlendPercent |
| Create Latent Task | UExLatentTask_Custom（Spawn + Activate） |
| Quest Task 等 | UExLatentTask_QuestBound |
| Async Load Asset / Stream Level / GameplayTag* | 对应 UExAsyncAction_* |

节点编译由 UExK2Node_AsyncBase（继承引擎 UK2Node_BaseAsyncTask）展开为 CreateProxy → SetK2NodeInfo → Activate，并注入 FExLatentNodeInfo。

---

## 运行时机制

### Proxy 创建（Flow 节点）

1. 从 WorldContext 取 UExLatentActionManager（UGameInstanceSubsystem）。  
2. 用 ObjectUUID + 节点 UUID 查 ProxyMap；已存在则复用，否则 NewObject 并注册。  
3. 多路 Exec 进入时递减 **InputCount**，归零后触发 OnBranchesFinished()（子类里启动 Timer / Tick / 条件检测）。

### Latent Task 生命周期

CreateTask / Spawn → **Pending** → Activate → **Running**（ReceiveOnStart、StartDelegate）→ TryStop → **Completed**（ReceiveOnStop、CompleteDelegate）→ 回收。  
Terminate → **Cancelled** 并 MarkAsGarbage。

### 关键类型

| 类型 | 作用 |
|------|------|
| UExLatentActionManager | ProxyMap：按 Key 存取 Flow Proxy，节点结束可 RemoveProxyObject |
| IExLatentTaskInterface | 状态与 TryStart / TryStop / Terminate 统一入口 |
| FExLatentNodeInfo | UUID、GraphNodeGuid、StartLog/EndLog、TimeOut（秒，0=禁用） |
| EExLatentTaskState | Pending / Running / Completed / Failed / Cancelled |

### 内存与 GC

- Proxy / Task 创建时常带 RF_StrongRefOnFrame，避免执行中被 GC。  
- AsyncAction 可用 RegisterWithGameInstance 挂到 UGameInstance。  
- 完成路径：SetReadyToDestroy、从 ProxyMap 移除、弱引用（TWeakObjectPtr）避免循环引用。

### 超时

SetK2NodeInfo 若 TimeOut > 0，启动 FTimer；到期且仍在 Running 则 TryStop()。

### 网络

UExBase_LatentTask::RunningState 带 ReplicatedUsing=OnRep_RunningState；需在合适 Authority 上创建/停止。Standalone 与 AutonomousProxy 下 IsLocal() 为 true。

---

## 扩展入口（C++）

| 目标 | 继承 | 必做 |
|------|------|------|
| 新流程节点 | UExBase_FlowProxy + UExK2Node_AsyncBase 子类 | 实现 OnBranchesFinished；K2 侧配置 Factory/Activate、ExpandNode |
| 新 Latent Task | UExBase_LatentTask 或 UExLatentTask_Saveable | 重写 OnStart/OnStop 或蓝图 ReceiveOnStart/ReceiveOnStop；结束调 TryStop |
| 新 AsyncAction | UExBase_AsyncAction | 工厂函数 + 委托；可选 UExK2Node_LatentTaskCall 注册专用节点 |
| Quest 联动 | UExLatentTask_QuestBound | 完成时写回 Quest 状态（见 Quest 文档） |

---

## 子系统与其它

| 组件 | 说明 |
|------|------|
| UExQuestManagerSubsystem | 任务状态、解锁链、存档（JSON V2） |
| UExBlueprintNodeGraphDebugSubsystem | 运行时调试辅助 |
| UExWorldPartitionSubsystem | 大世界分区相关工具 |
| UExBlueprintNodeLibrary / UExSaveGameLibrary | 蓝图与存档便捷 API |

---

## 延伸阅读

| 文档 | 内容 |
|------|------|
| [Usage.md](./Usage.md) | 节点参数、Latent Task 五步、FAQ |
| [QuestSystemGuide.md](./QuestSystemGuide.md) | Task / Objective / SubTask、DataAsset |
| [QuestDevPlan.md](./QuestDevPlan.md) | 阶段规划与 P3 |
