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
| UExBase_FlowProxy | UExProxy_LoopDelay、ForLoopWithDelay、WaitCondition、WaitBranch、BlendPercent | K2 创建，经 UExLatentActionManager 按 UUID 复用 |
| UExBase_LatentTask | Custom、Saveable、**Quest**（Quest 专用基类）、BranchSync（K2 内部） | TryStart / TryStop；Quest 子类可写回 Objective |
| UExBase_AsyncAction | LoadAsset、StreamLevel、GameplayTag*、网络/存档 | 委托驱动 |

**蓝图入口**：通用流程用 UExLatentTask_Custom / Saveable；**所有 Quest Latent 继承 UExLatentTask_Quest**，用 UExK2Node_QuestTask。

---

## 目录结构（Source/）

| 路径 | 内容 |
|------|------|
| .../Proxies/ | 流程代理 |
| .../LatentTasks/ | Latent 基类；**ExLatentTask_Quest.h** 为 Quest 基类 |
| .../AsyncActions/ | 异步 Action |
| .../Subsystems/ | LatentActionManager、调试等 |
| Public/Quest/ | Quest 子系统、DataAsset、UI |
| Editor/.../K2Nodes/ | UExK2Node_* |

---

## K2 节点 ↔ 运行时

| K2 节点 | 运行时 |
|---------|--------|
| DelayInLoop 等 Flow 节点 | UExProxy_* |
| Quest Task | **UExLatentTask_Quest**（CreateQuestProxy） |
| Create Latent Task | UExLatentTask_Custom（勿用于 Quest） |

编译链：UExK2Node_AsyncBase → CreateProxy → SetK2NodeInfo → Activate。

---

## 运行时要点

- **Proxy**：GameInstance 上 ProxyMap，InputCount 归零 → OnBranchesFinished。  
- **Latent Task**：Pending → Running → Completed；Quest 基类在 Completed 时可 ApplyQuestOnComplete（可 BlueprintNativeEvent 重写）。  
- **GC**：RF_StrongRefOnFrame；Async 可 RegisterWithGameInstance。  
- **网络**：RunningState 复制；Quest 写回走 Route API / ReplicationComponent。

---

## 扩展（C++）

| 目标 | 做法 |
|------|------|
| Quest 玩法 Latent | 继承 **UExLatentTask_Quest**，重写 ReceiveOnStart/Stop 或 ApplyQuestOnComplete |
| 通用 Latent | 继承 UExLatentTask_Custom / Saveable |
| 新 Flow 节点 | UExBase_FlowProxy + UExK2Node_AsyncBase |

---

## 延伸阅读

| 文档 | 内容 |
|------|------|
| [Usage.md](./Usage.md) | 节点速查、FAQ |
| [QuestSystemGuide.md](./QuestSystemGuide.md) | Task / Objective、联机 |
| [QuestDevPlan.md](./QuestDevPlan.md) | 阶段与验收 |
