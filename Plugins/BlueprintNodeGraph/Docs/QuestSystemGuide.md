# 任务系统使用指南

## 概述

BlueprintNodeGraph 插件提供层级任务系统，支持：

- **扁平存储 + 父子关系**：`AllTasks` 列表 + `ParentTaskId` / `SubTaskIds`
- **GameplayTag ID**：任务与目标均使用 `FGameplayTag`
- **目标进度**：多目标、可选目标、自动完成流转
- **前置任务**：`PreTaskIds` 在激活时校验
- **存档**：`SaveQuestProgress` / `LoadQuestProgress`
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

```cpp
#include "Quest/ExQuestManagerSubsystem.h"

if (UExQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UExQuestManagerSubsystem>())
{
	QuestManager->LoadQuestData(QuestData);
}
```

或使用蓝图：**Get Quest Manager** → **Load Quest Data**。

### 3. 激活与更新

```cpp
// 激活（会校验状态 + PreTaskIds 前置任务是否已完成）
QuestManager->ActivateQuest(FGameplayTag::RequestGameplayTag(FName("Quest.Main_001")));

// 更新目标进度（全部必选目标完成后自动将任务设为 Completed）
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
FString SaveData = QuestManager->SaveQuestProgress();
QuestManager->LoadQuestProgress(SaveData);
```

存档格式：

```
Quest.Main_001|1
  Quest.Main_001.Obj_001|1|1
  Quest.Main_001.Obj_002|0|0
```

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
| AllTasks | TArray\<FExQuestTask\> | 所有任务（扁平） |

查询辅助：`GetRootTasks()`、`GetSubTasks(ParentId)`、`CanActivateTask(TaskId)`。

---

## 任务状态

```cpp
enum class EExQuestState : uint8
{
	Inactive,   // 可激活
	Active,     // 进行中
	Completed,  // 已完成
	Failed,     // 失败
	Locked      // 锁定
};
```

---

## 与 BlueprintNodeGraph 延迟任务集成

任务系统与 LatentTask **无内置硬耦合**，可按需在蓝图/C++ 中桥接：

```cpp
#include "BlueprintTool/Proxies/ExProxy_LatentTask.h"

// K2 工厂：创建可 BP 继承的 Latent Task
UExProxy_LatentTask* TaskProxy = UExProxy_LatentTask::CreateProxy(
	WorldContextObject, MyLatentTaskClass);

// 绑定 UExBase_LatentTask 的委托
TaskProxy->CompleteDelegate.AddDynamic(this, &UMyClass::OnLatentTaskComplete);
TaskProxy->Activate();
```

自定义 Latent Task 应继承 `UExBase_LatentTask`（或 `UExProxy_LatentTask` 用于 K2 入口）。

---

## 蓝图常用节点

| 功能 | 节点 |
|------|------|
| 创建示例数据 | Create Example Quest Data |
| 获取 Manager | Get Quest Manager |
| 激活任务 | Activate Quest |
| 更新目标 | Update Quest Objective |
| 检查可激活 | Can Quest Activate With Data |
| 获取根任务 | Get Root Quests In Data |

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
