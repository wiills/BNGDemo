# 任务系统使用指南

## 概述

BlueprintNodeGraph 插件提供层级任务系统，支持：

- **扁平存储 + 父子关系**：`AllTasks` 列表 + `ParentTaskId` / `SubTaskIds`
- **GameplayTag ID**：任务与目标均使用 `FGameplayTag`
- **目标进度**：多目标、可选目标、自动完成流转
- **前置任务**：`PreTaskIds` 在激活时校验；前置完成后自动 `Locked` → `Inactive`
- **解锁**：`UnlockQuest` 将满足条件的 `Locked` 任务变为 `Inactive`
- **DataAsset 配置**：`UExQuestDataAsset` 存静态定义，运行时由 Manager 合并
- **存档**：默认 JSON `#ExQuestSaveV2`；兼容加载 V1 文本
- **UI**：`UExQuestTreeWidget` 自动同步 Subsystem 数据

---

## 快速开始

### 1. 创建任务数据

#### 方法一：使用蓝图函数库（推荐）

```cpp
#include "Quest/ExQuestBlueprintLibrary.h"

FExQuestData QuestData = UExQuestBlueprintLibrary::CreateExampleQuestData();
```

或在蓝图中调用 **Create Example Quest Data**。

#### 方法二：C++ 手动构建

```cpp
#include "Quest/ExQuestTypes.h"

FExQuestTask MainQuest;
MainQuest.TaskId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_001"));
MainQuest.TaskName = FText::FromString(TEXT("主线任务：拯救世界"));
MainQuest.State = EExQuestState::Inactive;

FExQuestObjective Obj1;
Obj1.ObjectiveId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_001.Obj_001"));
Obj1.Description = FText::FromString(TEXT("找到勇者之剑"));
Obj1.TargetProgress = 1;
MainQuest.Objectives.Add(Obj1);

FExQuestTask SubQuest;
SubQuest.TaskId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_001.Sub_001"));
SubQuest.ParentTaskId = MainQuest.TaskId;
SubQuest.State = EExQuestState::Locked;
MainQuest.SubTaskIds.AddTag(SubQuest.TaskId);

FExQuestData QuestData;
QuestData.QuestSetName = FText::FromString(TEXT("主线任务集"));
QuestData.AllTasks.Add(MainQuest);
QuestData.AllTasks.Add(SubQuest);
```

> **注意**：任务存储在 `AllTasks` 扁平数组中，子任务通过 `ParentTaskId` 关联，不是嵌套 `SubTasks` 数组。

### 2. 初始化任务管理器

#### 方式 A：运行时数据（代码/蓝图构建）

```cpp
#include "Quest/ExQuestManagerSubsystem.h"

if (UExQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UExQuestManagerSubsystem>())
{
	QuestManager->LoadQuestData(QuestData);
}
```

#### 方式 B：DataAsset（推荐策划配置）

1. 内容浏览器 → **右键** → **杂项** → **数据资产** → `ExQuestDataAsset`
2. 填写 `TaskDefinitions`（`InitialState`、目标、前置、父子关系）
3. 加载：

```cpp
#include "Quest/ExQuestDefinition.h"

QuestManager->LoadQuestFromAsset(MyQuestAsset, /*bPreserveRuntime*/ false);
// 热更资产且保留进度：
QuestManager->LoadQuestFromAsset(MyQuestAsset, true);
```

蓝图：**Load Quest From Asset**。

### 3. 激活与更新

```cpp
// 激活（仅 Inactive 可激活；会校验 PreTaskIds）
QuestManager->ActivateQuest(FGameplayTag::RequestGameplayTag(FName("Quest.Main_001")));

// 手动解锁 Locked 任务（前置已满足时）
QuestManager->UnlockQuest(FGameplayTag::RequestGameplayTag(FName("Quest.Main_002")));

// 增量更新目标
QuestManager->IncrementQuestObjective(TaskId, ObjectiveId, 1);

// 更新目标进度（全部必选目标完成后自动将任务设为 Completed，并解锁后续任务）
QuestManager->UpdateQuestObjective(
	FGameplayTag::RequestGameplayTag(FName("Quest.Main_001")),
	FGameplayTag::RequestGameplayTag(FName("Quest.Main_001.Obj_001")),
	1);

// 或直接完成目标
QuestManager->CompleteQuestObjective(
	FGameplayTag::RequestGameplayTag(FName("Quest.Main_001")),
	FGameplayTag::RequestGameplayTag(FName("Quest.Main_001.Obj_001")));
```

### 4. 事件绑定

```cpp
QuestManager->OnQuestStateChanged.AddDynamic(this, &UMyClass::HandleQuestStateChanged);
QuestManager->OnQuestProgressChanged.AddDynamic(this, &UMyClass::HandleQuestProgressChanged);
QuestManager->OnQuestObjectiveUpdated.AddDynamic(this, &UMyClass::HandleQuestObjectiveUpdated);
QuestManager->OnQuestDataLoaded.AddDynamic(this, &UMyClass::HandleQuestDataLoaded);
```

### 5. 任务树 UI

1. 创建继承自 `ExQuestTreeWidget` 的用户控件
2. 绑定组件：`QuestScrollBox`、`RootQuestContainer`、`TitleText`
3. 在关卡中：

```cpp
#include "Quest/ExQuestTreeWidget.h"

UExQuestTreeWidget* Widget = CreateWidget<UExQuestTreeWidget>(GetWorld(), WidgetClass);
Widget->AddToViewport();
Widget->SetQuestData(QuestManager->GetQuestData()); // 同时写入 Manager
```

`bAutoSyncFromManager` 默认为 `true`：Manager 状态变化时 UI 自动从 Subsystem 拉取最新数据。

### 6. 存档

```cpp
// 默认 JSON V2
FString SaveData = QuestManager->SaveQuestProgress();
QuestManager->LoadQuestProgress(SaveData);

// 仅运行时进度（不含文案，适合嵌入总存档）
FExQuestRuntimeState Runtime = QuestManager->GetRuntimeState();
QuestManager->ApplyRuntimeState(Runtime);

// 显式旧版文本 V1 导出
FString Legacy = QuestManager->SaveQuestProgressAsTextV1();
```

**JSON V2**（`SaveQuestProgress` / `SaveQuestProgressAsJson`）示例头：

```
#ExQuestSaveV2
{"Version":2,"QuestSetId":"ExampleQuestSet","Tasks":[...]}
```

**V1 文本**仍可通过 `LoadQuestProgress` 加载；`SaveQuestProgressAsTextV1` 可导出。

`LoadQuestProgress` / `LoadQuestProgressFromJson` 失败返回 `false`；成功广播 `OnQuestDataLoaded`。

---

## 数据结构

### FExQuestObjective

| 属性 | 类型 | 说明 |
|------|------|------|
| ObjectiveId | FGameplayTag | 目标 ID |
| Description | FText | 描述 |
| CurrentProgress | int32 | 当前进度 |
| TargetProgress | int32 | 目标进度 |
| bIsCompleted | bool | 是否完成 |
| bIsOptional | bool | 是否可选 |

### FExQuestTask

| 属性 | 类型 | 说明 |
|------|------|------|
| TaskId | FGameplayTag | 任务 ID |
| TaskName | FText | 名称 |
| State | EExQuestState | 状态 |
| Objectives | TArray | 目标列表 |
| SubTaskIds | FGameplayTagContainer | 子任务 ID（引用） |
| PreTaskIds | FGameplayTagContainer | 前置任务 ID |
| ParentTaskId | FGameplayTag | 父任务 ID |

### FExQuestData

| 属性 | 类型 | 说明 |
|------|------|------|
| QuestSetId | FString | 任务集 ID |
| QuestSetName | FText | 任务集名称 |
| AllTasks | TArray\<FExQuestTask\> | 所有任务（扁平，定义+运行时合并） |

查询辅助：`GetRootTasks()`、`GetSubTasks(ParentId)`、`CanActivateTask(TaskId)`、`RebuildIndices()`。

### FExQuestRuntimeState / UExQuestDataAsset

| 类型 | 用途 |
|------|------|
| `UExQuestDataAsset` | 策划配置：`TaskDefinitions`、`InitialState` |
| `FExQuestRuntimeState` | 仅状态与目标进度，可单独序列化 |
| `FExQuestTaskDefinition` | 单任务静态定义 → `ToRuntimeTask()` |

---

## 任务状态

```cpp
enum class EExQuestState : uint8
{
	Inactive,   // 可激活（ActivateQuest）
	Active,     // 进行中
	Completed,  // 已完成
	Failed,     // 失败
	Locked      // 锁定（需 UnlockQuest 或满足前置后自动解锁为 Inactive）
};
```

**状态流转要点**：

- `Locked` 不能直接 `ActivateQuest`，需先变为 `Inactive`
- 前置任务 `Completed` 后，依赖该前置的 `Locked` 任务自动变为 `Inactive`
- 父任务 `Completed` 后，子任务（`ParentTaskId`）若为 `Locked` 也会自动变为 `Inactive`
- `bIsRepeatable` 为 true 时，完成后重置为 `Inactive` 并清空目标进度

---

## 与 BlueprintNodeGraph 延迟任务集成

任务系统与 LatentTask **无内置硬耦合**，可按需在蓝图/C++ 中桥接：

```cpp
#include "BlueprintTool/LatentTasks/ExLatentTask_Custom.h"

// K2 工厂：创建可 BP 继承的 Latent Task
UExLatentTask_Custom* TaskProxy = UExLatentTask_Custom::CreateProxy(
	WorldContextObject, MyLatentTaskClass);

// 绑定 UExBase_LatentTask 的委托
TaskProxy->CompleteDelegate.AddDynamic(this, &UMyClass::OnLatentTaskComplete);
TaskProxy->Activate();
```

自定义 Latent Task 应继承 `UExLatentTask_Custom`（自定义）或 `UExLatentTask_Saveable`（需存档）。

---

## 蓝图常用节点

| 功能 | 节点 |
|------|------|
| 创建示例数据 | Create Example Quest Data |
| 获取 Manager | Get Quest Manager |
| 激活任务 | Activate Quest |
| 解锁任务 | Unlock Quest |
| 更新目标 | Update Quest Objective |
| 增量目标 | Increment Quest Objective |
| 检查可激活 | Can Quest Activate With Data |
| 检查可解锁 | Can Quest Unlock With Data |
| 获取根任务 | Get Root Quests In Data |
| 从资产加载 | Load Quest From Asset |
| 资产转数据 | Build Quest Data From Asset |
| JSON 存档 | Save / Load Quest Progress As Json |
| 提取运行时 | Extract Runtime State From Data |

---

## 最佳实践

1. **GameplayTag 命名**：`Quest.Chapter01.Main001`、`Quest.Chapter01.Main001.Obj_001`
2. **前置任务**：用 `Add Pre Task Id` 或 `PreTaskIds`，激活时自动校验
3. **层级**：用 `ParentTaskId` + `AllTasks`，避免嵌套 struct
4. **UI**：优先通过 Manager API 改状态，让 Widget 自动同步
5. **存档**：将 `SaveQuestProgress` 字符串并入游戏存档系统

---

## 示例资产

插件 Content 目录：

- `Content/Quest/WBP_QuestTree.uasset` — 任务树 UI 示例
- `Content/BP_QuestHost.uasset` — 任务宿主示例

示例代码见 `UExQuestBlueprintLibrary::CreateExampleQuestData()`。
