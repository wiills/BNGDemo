# BlueprintNodeGraph 使用指南

> 编码：UTF-8。侧重**蓝图流程节点**与 **Latent Task**；任务系统（Quest）见 [QuestSystemGuide.md](./QuestSystemGuide.md)，类层次见 [Architecture.md](./Architecture.md)。

---

## 快速开始

1. 将 BlueprintNodeGraph 放入 项目/Plugins/，重启引擎并启用插件。
2. 在蓝图图表中搜索 **Blueprint Node Graph** 分类下的节点（见下表）。
3. 需要可复用、可序列化的多步逻辑时，继承 **ExLatentTask_Custom**（或需存档时用 **ExLatentTask_Saveable**）创建任务蓝图。

**最小示例**：BeginPlay → **DelayInLoop**（Duration=1）→ Loop 引脚打印 → Completed 引脚结束。

---

## 蓝图节点速查

| 编辑器名称 | 用途 | 主要参数 | 输出引脚 |
|------------|------|----------|----------|
| **DelayInLoop** | 按间隔循环 N 次 | Duration、Count、Need First Delay | Loop、Completed |
| **For Loop With Delay** | 带索引的间隔循环 | Loop Count、Loop Interval、Need First Delay | Loop Body (Index)、Completed |
| **Wait Condition** | 多路输入汇合后，等待 Bool 为 true | Condition（引用） | Completed |
| **AsyncBlendPercent** | 多路输入后按混合百分比推进 | Percent Speed 1/2（引用） | Completed |
| **Wait All** | 所有输入分支完成 | （多 Exec 输入） | Completed |
| **Wait Any** | 任一分支完成 | （多 Exec 输入） | Completed |
| **Wait Count** | 收到 N 次触发 | Count | Completed |
| **Create Latent Task** | 创建并运行自定义任务类 | Class、任务参数 | OnStart、OnComplete、Then |

**注意**

- 异步节点至少会有 **1 帧延迟**（DelayInLoop 等基于 Timer，非同步 Tick）。
- 节点 Details 中的 **Node Info** 可配 StartLog / EndLog / TimeOut（秒，0=不超时）。
- 与 **Quest** 相关 Latent 请继承 UExLatentTask_Quest 并用 Quest Task 节点，勿与普通 ExLatentTask_Custom 混用。

---

## 自定义 Latent Task（5 步）

| 步骤 | 操作 |
|------|------|
| 1 | 新建蓝图类，父类选 **ExLatentTask_Custom**（需存档选 **ExLatentTask_Saveable**） |
| 2 | 在任务蓝图实现 **Receive On Start** / **Receive On Stop**（清理定时器、解绑事件） |
| 3 | 逻辑结束时调用 **Try Stop**（进入 Completed，触发 OnComplete） |
| 4 | 需要参数时在任务蓝图添加变量，由 **Create Latent Task** 节点写入 |
| 5 | 调用方：Create Latent Task → 绑定 **OnStart** / **OnComplete** → **Activate** |

**调用方示意**

`
Event BeginPlay
  → Create Latent Task (你的任务类)
      ├ On Start   → …
      └ On Complete → …
  → Activate
`

**生命周期**：Pending → Running → Completed；Terminate 为 Cancelled 并标记回收。

父类列表中仅应出现 ExLatentTask_Custom / ExLatentTask_Saveable；内部基类已 NotBlueprintType，不会出现在 Reparent 中。

---

## C++ 要点

`cpp
UExBase_LatentTask* Task = UExBase_LatentTask::CreateTask(WorldContext, TaskClass);
Task->SetK2NodeInfo(NodeInfo);
Task->CompleteDelegate.AddDynamic(this, &UMyClass::OnComplete);
Task->Activate();

// 控制
Task->GetState();   // EExLatentTaskState
Task->TryStop();    // 正常结束
Task->Terminate();  // 取消
`

状态枚举：Pending / Running / Completed / Failed / Cancelled。

---

## 调试与常见问题

| 现象 | 排查 |
|------|------|
| 节点不执行 | World Context 是否有效；是否在服务端/客户端预期环境 |
| 崩溃 | 对 Actor/对象使用 IsValid |
| 多人不同步 | Latent Task 默认复制 RunningState，确认 Actor Replicates |
| 被 GC | 长生命周期任务确认已 Activate；异步 Action 侧可用 RegisterWithGameInstance |

日志：Output Log 中按 Node Info 的 **StartLog** / **EndLog** 过滤；类别 LogLatentTask、LogAsyncAction。

---

## 示例资产

`
Content/
├── BP_TestBlueprintNodes.uasset   # 节点示例
└── Tasks/
    ├── BP_TestTask.uasset
    └── BP_TestTask2.uasset
`

---


---

## 任务 DataTable → DA（编辑器导入）

> 完整流程见 [QuestMapFlowExample.md](./QuestMapFlowExample.md)。**保存 DataTable 不会自动更新 DA**，须执行导入。

| 步骤 | 操作 |
|------|------|
| 1 | 新建 **DataTable**，Row Type = **FExQuestTaskTableRow** |
| 2 | 填写行（TaskId 须为有效 GameplayTag） |
| 3 | 保存表（Ctrl+S） |
| 4 | 右键表 → **Import To Paired Quest Data Asset** |
| 5 | 同目录 **DA_Quest_***（DT_Quest_Test → DA_Quest_Test） |
| 6 | 改表后重复步骤 4，或 DA 上 **Import From Source Task Table** |

- 命名：DT_ → DA_；导入为拷贝到 TaskDefinitions，SourceTaskTable 记来源。
- 运行时只 **Load DA**（Agent），不读表。
- 成功：右下角 **绿色通知**。

## 延伸阅读

| 文档 | 内容 |
|------|------|
| [Architecture.md](./Architecture.md) | 模块划分、Proxy / LatentTask 类图、编译流程 |
| [QuestSystemGuide.md](./QuestSystemGuide.md) | Task / Objective / SubTask、DataAsset、存档 |
| [QuestDevPlan.md](./QuestDevPlan.md) | 开发阶段与 P3 待办 |
