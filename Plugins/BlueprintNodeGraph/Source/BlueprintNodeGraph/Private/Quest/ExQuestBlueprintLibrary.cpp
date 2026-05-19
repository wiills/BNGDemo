// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestBlueprintLibrary.h"
#include "Quest/ExQuestManagerSubsystem.h"
#include "Quest/ExQuestDefinition.h"
#include "Kismet/GameplayStatics.h"

FExQuestData UExQuestBlueprintLibrary::CreateExampleQuestData()
{
	FExQuestData QuestData;
	QuestData.QuestSetId = TEXT("ExampleQuestSet");
	QuestData.QuestSetName = FText::FromString(TEXT("示例任务集"));

	FExQuestTask MainQuest;
	MainQuest.TaskId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_001"));
	MainQuest.TaskName = FText::FromString(TEXT("主线任务：拯救世界"));
	MainQuest.Description = FText::FromString(TEXT("拯救世界于危难之中"));
	MainQuest.State = EExQuestState::Inactive;

	FExQuestObjective Obj1;
	Obj1.ObjectiveId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_001.Obj_001"));
	Obj1.Description = FText::FromString(TEXT("找到勇者之剑"));
	Obj1.TargetProgress = 1;
	MainQuest.Objectives.Add(Obj1);

	FExQuestObjective Obj2;
	Obj2.ObjectiveId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_001.Obj_002"));
	Obj2.Description = FText::FromString(TEXT("击败魔王"));
	Obj2.TargetProgress = 1;
	MainQuest.Objectives.Add(Obj2);

	FExQuestTask SubQuest;
	SubQuest.TaskId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_001.Sub_001"));
	SubQuest.TaskName = FText::FromString(TEXT("支线任务：收集装备"));
	SubQuest.Description = FText::FromString(TEXT("收集强力装备"));
	SubQuest.ParentTaskId = MainQuest.TaskId;
	SubQuest.State = EExQuestState::Locked;

	FExQuestObjective SubObj1;
	SubObj1.ObjectiveId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_001.Sub_001.Obj_001"));
	SubObj1.Description = FText::FromString(TEXT("收集10把武器"));
	SubObj1.TargetProgress = 10;
	SubQuest.Objectives.Add(SubObj1);

	FExQuestObjective SubObj2;
	SubObj2.ObjectiveId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_001.Sub_001.Obj_002"));
	SubObj2.Description = FText::FromString(TEXT("收集5套盔甲"));
	SubObj2.TargetProgress = 5;
	SubObj2.bIsOptional = true;
	SubQuest.Objectives.Add(SubObj2);

	MainQuest.SubTaskIds.AddTag(SubQuest.TaskId);

	FExQuestTask MainQuest2;
	MainQuest2.TaskId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_002"));
	MainQuest2.TaskName = FText::FromString(TEXT("主线任务：恢复和平"));
	MainQuest2.Description = FText::FromString(TEXT("让世界恢复和平"));
	MainQuest2.State = EExQuestState::Locked;
	MainQuest2.PreTaskIds.AddTag(MainQuest.TaskId);

	FExQuestObjective Main2Obj1;
	Main2Obj1.ObjectiveId = FGameplayTag::RequestGameplayTag(FName("Quest.Main_002.Obj_001"));
	Main2Obj1.Description = FText::FromString(TEXT("帮助村民重建家园"));
	Main2Obj1.TargetProgress = 1;
	MainQuest2.Objectives.Add(Main2Obj1);

	QuestData.AllTasks.Add(MainQuest);
	QuestData.AllTasks.Add(SubQuest);
	QuestData.AllTasks.Add(MainQuest2);
	QuestData.RebuildIndices();

	return QuestData;
}

FExQuestObjective UExQuestBlueprintLibrary::CreateQuestObjective(
	const FGameplayTag& ObjectiveId,
	const FText& Description,
	int32 TargetProgress,
	bool bIsOptional)
{
	FExQuestObjective Objective;
	Objective.ObjectiveId = ObjectiveId;
	Objective.Description = Description;
	Objective.TargetProgress = TargetProgress;
	Objective.bIsOptional = bIsOptional;
	Objective.CurrentProgress = 0;
	Objective.bIsCompleted = false;
	return Objective;
}

FExQuestTask UExQuestBlueprintLibrary::CreateQuestTask(
	const FGameplayTag& TaskId,
	const FText& TaskName,
	const FText& Description,
	EExQuestState InitialState)
{
	FExQuestTask Task;
	Task.TaskId = TaskId;
	Task.TaskName = TaskName;
	Task.Description = Description;
	Task.State = InitialState;
	Task.bIsRepeatable = false;
	return Task;
}

FExQuestData UExQuestBlueprintLibrary::CreateQuestData(
	const FText& QuestSetName,
	const TArray<FExQuestTask>& AllTasks)
{
	FExQuestData QuestData;
	QuestData.QuestSetId = FGuid::NewGuid().ToString();
	QuestData.QuestSetName = QuestSetName;
	QuestData.AllTasks = AllTasks;
	QuestData.RebuildIndices();
	return QuestData;
}

FExQuestTask UExQuestBlueprintLibrary::AddSubTaskId(FExQuestTask Task, const FGameplayTag& SubTaskId)
{
	Task.SubTaskIds.AddTag(SubTaskId);
	return Task;
}

FExQuestTask UExQuestBlueprintLibrary::AddPreTaskId(FExQuestTask Task, const FGameplayTag& PreTaskId)
{
	Task.PreTaskIds.AddTag(PreTaskId);
	return Task;
}

FExQuestTask UExQuestBlueprintLibrary::AddObjectiveToQuest(FExQuestTask Task, const FExQuestObjective& Objective)
{
	Task.Objectives.Add(Objective);
	return Task;
}

bool UExQuestBlueprintLibrary::FindQuestInData(const FExQuestData& QuestData, const FGameplayTag& TaskId, FExQuestTask& OutTask)
{
	return QuestData.FindTaskById(TaskId, OutTask);
}

TArray<FExQuestTask> UExQuestBlueprintLibrary::GetRootQuestsInData(const FExQuestData& QuestData)
{
	return QuestData.GetRootTasks();
}

TArray<FExQuestTask> UExQuestBlueprintLibrary::GetSubQuestsInData(const FExQuestData& QuestData, const FGameplayTag& ParentTaskId)
{
	return QuestData.GetSubTasks(ParentTaskId);
}

TArray<FExQuestTask> UExQuestBlueprintLibrary::GetAllActiveQuestsInData(const FExQuestData& QuestData)
{
	return QuestData.GetAllActiveTasks();
}

TArray<FExQuestTask> UExQuestBlueprintLibrary::GetAllCompletedQuestsInData(const FExQuestData& QuestData)
{
	return QuestData.GetAllCompletedTasks();
}

float UExQuestBlueprintLibrary::GetQuestCompletionPercent(const FExQuestTask& Task)
{
	return Task.GetCompletionPercent();
}

bool UExQuestBlueprintLibrary::IsQuestFullyCompleted(const FExQuestTask& Task)
{
	return Task.IsFullyCompleted();
}

bool UExQuestBlueprintLibrary::CanQuestUnlockWithData(const FExQuestData& QuestData, const FGameplayTag& TaskId)
{
	FExQuestTask Task;
	if (!QuestData.FindTaskById(TaskId, Task) || !Task.CanUnlock())
	{
		return false;
	}
	return Task.ArePreTasksSatisfied(QuestData);
}

bool UExQuestBlueprintLibrary::CanQuestActivate(const FExQuestTask& Task)
{
	return Task.CanActivate();
}

bool UExQuestBlueprintLibrary::CanQuestActivateWithData(const FExQuestData& QuestData, const FGameplayTag& TaskId)
{
	return QuestData.CanActivateTask(TaskId);
}

bool UExQuestBlueprintLibrary::IsQuestLocked(const FExQuestTask& Task)
{
	return Task.State == EExQuestState::Locked;
}

bool UExQuestBlueprintLibrary::IsQuestFailed(const FExQuestTask& Task)
{
	return Task.State == EExQuestState::Failed;
}

bool UExQuestBlueprintLibrary::IsQuestInactive(const FExQuestTask& Task)
{
	return Task.State == EExQuestState::Inactive;
}

FText UExQuestBlueprintLibrary::GetQuestStateText(EExQuestState State)
{
	switch (State)
	{
	case EExQuestState::Inactive:
		return NSLOCTEXT("QuestUI", "StateInactive", "未激活");
	case EExQuestState::Active:
		return NSLOCTEXT("QuestUI", "StateActive", "进行中");
	case EExQuestState::Completed:
		return NSLOCTEXT("QuestUI", "StateCompleted", "已完成");
	case EExQuestState::Failed:
		return NSLOCTEXT("QuestUI", "StateFailed", "失败");
	case EExQuestState::Locked:
		return NSLOCTEXT("QuestUI", "StateLocked", "已锁定");
	default:
		return NSLOCTEXT("QuestUI", "StateUnknown", "未知");
	}
}

FLinearColor UExQuestBlueprintLibrary::GetQuestStateColor(EExQuestState State)
{
	switch (State)
	{
	case EExQuestState::Active:
		return FLinearColor::Yellow;
	case EExQuestState::Completed:
		return FLinearColor::Green;
	case EExQuestState::Failed:
		return FLinearColor::Red;
	case EExQuestState::Locked:
		return FLinearColor::Gray;
	default:
		return FLinearColor::White;
	}
}

UExQuestManagerSubsystem* UExQuestBlueprintLibrary::GetQuestManager(UObject* WorldContextObject)
{
	if (UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		return GameInstance->GetSubsystem<UExQuestManagerSubsystem>();
	}
	return nullptr;
}

bool UExQuestBlueprintLibrary::UnlockQuest(UObject* WorldContextObject, const FGameplayTag& TaskId)
{
	if (UExQuestManagerSubsystem* QuestManager = GetQuestManager(WorldContextObject))
	{
		return QuestManager->UnlockQuest(TaskId);
	}
	return false;
}

bool UExQuestBlueprintLibrary::IncrementQuestObjective(UObject* WorldContextObject, const FGameplayTag& TaskId, const FGameplayTag& ObjectiveId, int32 Delta)
{
	if (UExQuestManagerSubsystem* QuestManager = GetQuestManager(WorldContextObject))
	{
		return QuestManager->IncrementQuestObjective(TaskId, ObjectiveId, Delta);
	}
	return false;
}

void UExQuestBlueprintLibrary::LoadQuestFromAsset(UObject* WorldContextObject, UExQuestDataAsset* QuestAsset, bool bPreserveRuntime)
{
	if (UExQuestManagerSubsystem* QuestManager = GetQuestManager(WorldContextObject))
	{
		QuestManager->LoadQuestFromAsset(QuestAsset, bPreserveRuntime);
	}
}

FExQuestData UExQuestBlueprintLibrary::BuildQuestDataFromAsset(const UExQuestDataAsset* QuestAsset)
{
	if (QuestAsset)
	{
		return QuestAsset->BuildInitialQuestData();
	}
	return FExQuestData();
}

FString UExQuestBlueprintLibrary::SaveQuestProgressAsJson(UObject* WorldContextObject)
{
	if (const UExQuestManagerSubsystem* QuestManager = GetQuestManager(WorldContextObject))
	{
		return QuestManager->SaveQuestProgressAsJson();
	}
	return FString();
}

bool UExQuestBlueprintLibrary::LoadQuestProgressFromJson(UObject* WorldContextObject, const FString& JsonSaveData)
{
	if (UExQuestManagerSubsystem* QuestManager = GetQuestManager(WorldContextObject))
	{
		return QuestManager->LoadQuestProgressFromJson(JsonSaveData);
	}
	return false;
}

FExQuestRuntimeState UExQuestBlueprintLibrary::ExtractRuntimeStateFromData(const FExQuestData& QuestData)
{
	return QuestData.ExtractRuntimeState();
}

void UExQuestBlueprintLibrary::ApplyRuntimeStateToManager(UObject* WorldContextObject, const FExQuestRuntimeState& RuntimeState)
{
	if (UExQuestManagerSubsystem* QuestManager = GetQuestManager(WorldContextObject))
	{
		QuestManager->ApplyRuntimeState(RuntimeState);
	}
}

FGameplayTag UExQuestBlueprintLibrary::MakeQuestTag(const FString& TagString)
{
	return FGameplayTag::RequestGameplayTag(FName(*TagString));
}

bool UExQuestBlueprintLibrary::HasSubTasks(const FExQuestTask& Task)
{
	return Task.SubTaskIds.Num() > 0;
}

bool UExQuestBlueprintLibrary::HasPreTasks(const FExQuestTask& Task)
{
	return Task.PreTaskIds.Num() > 0;
}

int32 UExQuestBlueprintLibrary::GetQuestCount(const FExQuestData& QuestData)
{
	return QuestData.AllTasks.Num();
}
