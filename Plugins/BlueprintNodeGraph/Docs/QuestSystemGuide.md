# 任务系统使用指南

## 概述

BlueprintNodeGraph 插件提供了一个完整的层级任务系统，支持：

- **层级任务树**：支持嵌套的子任务结构
- **任务状态管理**：锁定、激活、完成、失败等状态
- **目标进度系统**：支持多目标的进度追踪
- **存档功能**：保存和加载任务进度
- **UI 组件**：任务树显示组件
- **事件系统**：任务状态变化通知

## 快速开始

### 1. 创建任务数据

#### 方法一：在 C++ 中创建

```cpp
#include "BlueprintTool/ExQuestTypes.h"

// 创建根任务
FExQuestTask MainQuest;
MainQuest.TaskId = TEXT("Main_Quest_001");
MainQuest.TaskName = FText::FromString(TEXT("主线任务：拯救世界"));
MainQuest.Description = FText::FromString(TEXT("拯救世界于危难之中"));
MainQuest.State = EExQuestState::Locked;

// 添加目标
FExQuestObjective Objective1;
Objective1.ObjectiveId = TEXT("Obj_001");
Objective1.Description = FText::FromString(TEXT("找到勇者之剑"));
Objective1.TargetProgress = 1;
MainQuest.Objectives.Add(Objective1);

FExQuestObjective Objective2;
Objective2.ObjectiveId = TEXT("Obj_002");
Objective2.Description = FText::FromString(TEXT("击败魔王"));
Objective2.TargetProgress = 1;
MainQuest.Objectives.Add(Objective2);

// 添加子任务
FExQuestTask SubQuest;
SubQuest.TaskId = TEXT("Sub_Quest_001");
SubQuest.TaskName = FText::FromString(TEXT("支线任务：收集装备"));
SubQuest.ParentTaskId = MainQuest.TaskId;
SubQuest.State = EExQuestState::Locked;

// 创建任务数据
FExQuestData QuestData;
QuestData.QuestSetId = TEXT("MainQuestSet");
QuestData.QuestSetName = FText::FromString(TEXT("主线任务集"));
QuestData.RootTasks.Add(MainQuest);
```

#### 方法二：在蓝图中创建

直接使用 `Make ExQuestTask` 和相关节点来构建任务数据。

### 2. 初始化任务系统

在游戏实例或 GameMode 中：

```cpp
#include "BlueprintTool/ExQuestManagerSubsystem.h"

// 获取任务管理器
if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
{
	UExQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UExQuestManagerSubsystem>();

	// 加载任务数据
	QuestManager->LoadQuestData(QuestData);
}
```

### 3. 使用任务管理器

#### 激活任务

```cpp
QuestManager->ActivateQuest(TEXT("Main_Quest_001"));
```

#### 更新任务目标进度

```cpp
// 增加进度
QuestManager->UpdateQuestObjective(TEXT("Main_Quest_001"), TEXT("Obj_001"), 1);

// 或者直接完成目标
QuestManager->CompleteQuestObjective(TEXT("Main_Quest_001"), TEXT("Obj_001"));
```

#### 完成任务

```cpp
QuestManager->CompleteQuest(TEXT("Main_Quest_001"));
```

#### 获取任务信息

```cpp
FExQuestTask OutTask;
if (QuestManager->GetQuestById(TEXT("Main_Quest_001"), OutTask))
{
	float CompletionPercent = OutTask.GetCompletionPercent();
	EExQuestState State = OutTask.State;
}
```

### 4. 绑定事件监听

```cpp
// 绑定任务状态变化事件
QuestManager->OnQuestStateChanged.AddDynamic(this, &UMyClass::OnQuestStateChanged);

// 绑定任务进度变化事件
QuestManager->OnQuestProgressChanged.AddDynamic(this, &UMyClass::OnQuestProgressChanged);

// 绑定目标更新事件
QuestManager->OnQuestObjectiveUpdated.AddDynamic(this, &UMyClass::OnQuestObjectiveUpdated);
```

### 5. 使用任务树 UI

#### 创建 UI 蓝图

1. 创建继承自 `ExQuestTreeWidget` 的用户控件蓝图
2. 在编辑器中设计 UI：
   - 添加 `ScrollBox` 组件命名为 `QuestScrollBox`
   - 在其中添加 `VerticalBox` 组件命名为 `RootQuestContainer`
   - 添加 `TextBlock` 组件命名为 `TitleText`

#### 在关卡中显示 UI

```cpp
// 创建并显示任务树 UI
UExQuestTreeWidget* QuestTreeWidget = CreateWidget<UExQuestTreeWidget>(GetWorld(), QuestTreeWidgetClass);
if (QuestTreeWidget)
{
	QuestTreeWidget->AddToViewport();

	// 设置任务数据
	QuestTreeWidget->SetQuestData(QuestManager->GetQuestData());
}
```

### 6. 存档和加载

```cpp
// 保存任务进度
FString SaveData = QuestManager->SaveQuestProgress();

// 加载任务进度
QuestManager->LoadQuestProgress(SaveData);
```

## 任务结构详解

### FExQuestObjective - 任务目标

| 属性 | 类型 | 说明 |
|------|------|------|
| ObjectiveId | FString | 目标唯一 ID |
| Description | FText | 目标描述 |
| CurrentProgress | int32 | 当前进度 |
| TargetProgress | int32 | 目标进度 |
| bIsCompleted | bool | 是否完成 |
| bIsOptional | bool | 是否可选 |

### FExQuestTask - 任务

| 属性 | 类型 | 说明 |
|------|------|------|
| TaskId | FString | 任务唯一 ID |
| TaskName | FText | 任务名称 |
| Description | FText | 任务描述 |
| State | EExQuestState | 任务状态 |
| Objectives | TArray<FExQuestObjective> | 任务目标列表 |
| SubTasks | TArray<FExQuestTask> | 子任务列表 |
| ParentTaskId | FString | 父任务 ID |
| PreTaskIds | FGameplayTagContainer | 前置任务 ID 列表 |
| bIsRepeatable | bool | 是否可重复 |

### FExQuestData - 任务数据

| 属性 | 类型 | 说明 |
|------|------|------|
| QuestSetId | FString | 任务集 ID |
| QuestSetName | FText | 任务集名称 |
| RootTasks | TArray<FExQuestTask> | 根任务列表 |

## 任务状态

```cpp
enum class EExQuestState : uint8
{
	Inactive,   // 未激活
	Active,     // 进行中
	Completed,  // 已完成
	Failed,     // 失败
	Locked      // 已锁定
};
```

## 与 BlueprintNodeGraph 集成

您可以将任务系统与 BlueprintNodeGraph 的延迟任务系统集成：

```cpp
// 创建任务相关的延迟任务
UExLatentTaskProxy* QuestTask = CreateLatentTask(World, TaskClass);

// 在任务完成时调用蓝图节点
QuestTask->OnComplete.AddDynamic(this, &UMyClass::OnQuestTaskCompleted);
```

## 示例：完整任务流程

```cpp
// 1. 创建任务数据
FExQuestData QuestData = CreateExampleQuestData();

// 2. 加载到任务管理器
QuestManager->LoadQuestData(QuestData);

// 3. 绑定事件
QuestManager->OnQuestStateChanged.AddUObject(this, &UMyGameMode::OnQuestStateChanged);

// 4. 激活任务
QuestManager->ActivateQuest(TEXT("Main_Quest_001"));

// 5. 模拟玩家完成目标
QuestManager->UpdateQuestObjective(TEXT("Main_Quest_001"), TEXT("Obj_001"), 1);

// 6. 显示任务树 UI
ShowQuestTreeUI();
```

## 蓝图使用示例

### 在蓝图中创建任务数据

1. 使用 `Make ExQuestTask` 节点创建任务
2. 添加到 `Make ExQuestData` 的 `RootTasks` 数组
3. 调用 `Load Quest Data` 加载到任务管理器

### 在蓝图中管理任务

1. 获取 `Quest Manager Subsystem`
2. 调用 `Activate Quest`、`Complete Quest` 等节点
3. 绑定事件监听状态变化

### 在蓝图中创建任务树 UI

1. 创建继承自 `ExQuestTreeWidget` 的用户控件
2. 设计 UI 布局
3. 在关卡蓝图中创建并显示 UI

## 最佳实践

1. **任务 ID 命名规范**：使用清晰的命名规则，如 `Main_Chapter01_Quest001`
2. **目标设置**：将复杂任务拆分为多个小目标，提供更好的反馈
3. **任务树设计**：合理组织任务层级，避免过深的嵌套
4. **状态管理**：确保任务状态正确流转
5. **存档集成**：将任务进度与游戏存档系统集成
6. **UI 反馈**：提供清晰的任务状态视觉反馈
