// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestManagerSubsystem.h"
#include "Quest/ExQuestDefinition.h"
#include "Quest/ExQuestSave.h"

namespace ExQuestSaveFormat
{
	static const TCHAR* TextV1Header = TEXT("#ExQuestSaveV1");
}

void UExQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UExQuestManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UExQuestManagerSubsystem::SyncRuntimeStateCache()
{
	CachedRuntimeState = CurrentQuestData.ExtractRuntimeState();
}

void UExQuestManagerSubsystem::CaptureInitialStates()
{
	InitialTaskStates.Empty();
	for (const FExQuestTask& Task : CurrentQuestData.AllTasks)
	{
		if (Task.TaskId.IsValid())
		{
			InitialTaskStates.Add(Task.TaskId, Task.State);
		}
	}
}

void UExQuestManagerSubsystem::BroadcastTaskStateChange(const FExQuestTask& Task)
{
	OnQuestStateChanged.Broadcast(Task);
}

void UExQuestManagerSubsystem::BroadcastTaskProgress(const FExQuestTask& Task)
{
	OnQuestProgressChanged.Broadcast(Task.TaskId, Task.GetCompletionPercent());
}

void UExQuestManagerSubsystem::NotifyQuestDataRefreshed()
{
	SyncRuntimeStateCache();
	OnQuestDataLoaded.Broadcast();
}

void UExQuestManagerSubsystem::LoadQuestData(const FExQuestData& QuestData)
{
	CurrentQuestData = QuestData;
	CurrentQuestData.RebuildIndices();
	LoadedQuestAsset = nullptr;
	CaptureInitialStates();
	NotifyQuestDataRefreshed();
}

void UExQuestManagerSubsystem::LoadQuestFromAsset(UExQuestDataAsset* QuestAsset, bool bPreserveRuntime)
{
	if (!QuestAsset)
	{
		return;
	}

	FExQuestRuntimeState PreviousRuntime;
	if (bPreserveRuntime)
	{
		PreviousRuntime = CachedRuntimeState.QuestSetId.IsEmpty()
			? CurrentQuestData.ExtractRuntimeState()
			: CachedRuntimeState;
	}

	FExQuestData NewData = QuestAsset->BuildInitialQuestData();

	if (bPreserveRuntime && !PreviousRuntime.QuestSetId.IsEmpty() && PreviousRuntime.QuestSetId == NewData.QuestSetId)
	{
		NewData.ApplyRuntimeState(PreviousRuntime);
	}

	LoadedQuestAsset = QuestAsset;
	CurrentQuestData = MoveTemp(NewData);
	CurrentQuestData.RebuildIndices();
	CaptureInitialStates();
	NotifyQuestDataRefreshed();
}

FExQuestRuntimeState UExQuestManagerSubsystem::GetRuntimeState() const
{
	return CurrentQuestData.ExtractRuntimeState();
}

void UExQuestManagerSubsystem::ApplyRuntimeState(const FExQuestRuntimeState& RuntimeState)
{
	CurrentQuestData.ApplyRuntimeState(RuntimeState);
	CurrentQuestData.RebuildIndices();
	NotifyQuestDataRefreshed();
}

bool UExQuestManagerSubsystem::TryUnlockTask(FExQuestTask& Task)
{
	if (!Task.CanUnlock())
	{
		return false;
	}

	if (!Task.ArePreTasksSatisfied(CurrentQuestData))
	{
		return false;
	}

	Task.State = EExQuestState::Inactive;
	BroadcastTaskStateChange(Task);
	return true;
}

void UExQuestManagerSubsystem::UnlockDependentQuests(const FGameplayTag& CompletedTaskId)
{
	const TArray<FGameplayTag> DependentIds = CurrentQuestData.GetTaskIdsWithPreTask(CompletedTaskId);
	for (const FGameplayTag& DependentId : DependentIds)
	{
		FindAndUpdateTask(DependentId, [this](FExQuestTask& Task) -> bool
		{
			TryUnlockTask(Task);
			return true;
		});
	}

	const TArray<FExQuestTask> SubTasks = CurrentQuestData.GetSubTasks(CompletedTaskId);
	for (const FExQuestTask& SubTask : SubTasks)
	{
		FindAndUpdateTask(SubTask.TaskId, [this](FExQuestTask& Task) -> bool
		{
			if (Task.CanUnlock())
			{
				TryUnlockTask(Task);
			}
			return true;
		});
	}
}

void UExQuestManagerSubsystem::ResetTaskObjectives(FExQuestTask& Task)
{
	for (FExQuestObjective& Objective : Task.Objectives)
	{
		Objective.bIsCompleted = false;
		Objective.CurrentProgress = 0;
	}
}

void UExQuestManagerSubsystem::HandleQuestCompleted(FExQuestTask& Task)
{
	if (Task.bIsRepeatable)
	{
		Task.State = EExQuestState::Inactive;
		ResetTaskObjectives(Task);
		BroadcastTaskStateChange(Task);
		BroadcastTaskProgress(Task);
		SyncRuntimeStateCache();
		return;
	}

	UnlockDependentQuests(Task.TaskId);
	SyncRuntimeStateCache();
}

bool UExQuestManagerSubsystem::UnlockQuest(const FGameplayTag& TaskId)
{
	bool bUnlocked = false;
	FindAndUpdateTask(TaskId, [this, &bUnlocked](FExQuestTask& Task) -> bool
	{
		bUnlocked = TryUnlockTask(Task);
		return bUnlocked;
	});
	if (bUnlocked)
	{
		SyncRuntimeStateCache();
	}
	return bUnlocked;
}

bool UExQuestManagerSubsystem::ActivateQuest(const FGameplayTag& TaskId)
{
	if (!CurrentQuestData.CanActivateTask(TaskId))
	{
		return false;
	}

	const bool bResult = FindAndUpdateTask(TaskId, [this](FExQuestTask& Task) -> bool
	{
		Task.State = EExQuestState::Active;
		BroadcastTaskStateChange(Task);
		BroadcastTaskProgress(Task);
		return true;
	});

	if (bResult)
	{
		SyncRuntimeStateCache();
	}
	return bResult;
}

bool UExQuestManagerSubsystem::CompleteQuest(const FGameplayTag& TaskId)
{
	bool bCompleted = false;
	FindAndUpdateTask(TaskId, [this, &bCompleted](FExQuestTask& Task) -> bool
	{
		if (Task.State != EExQuestState::Active)
		{
			return false;
		}

		Task.State = EExQuestState::Completed;
		BroadcastTaskStateChange(Task);
		BroadcastTaskProgress(Task);
		HandleQuestCompleted(Task);
		bCompleted = true;
		return true;
	});
	return bCompleted;
}

bool UExQuestManagerSubsystem::FailQuest(const FGameplayTag& TaskId)
{
	const bool bResult = FindAndUpdateTask(TaskId, [this](FExQuestTask& Task) -> bool
	{
		if (Task.State == EExQuestState::Active)
		{
			Task.State = EExQuestState::Failed;
			BroadcastTaskStateChange(Task);
			return true;
		}
		return false;
	});

	if (bResult)
	{
		SyncRuntimeStateCache();
	}
	return bResult;
}

bool UExQuestManagerSubsystem::ApplyObjectiveProgress(FExQuestTask& Task, const FGameplayTag& ObjectiveId, int32 NewProgress)
{
	for (FExQuestObjective& Objective : Task.Objectives)
	{
		if (Objective.ObjectiveId != ObjectiveId)
		{
			continue;
		}

		Objective.CurrentProgress = FMath::Clamp(NewProgress, 0, Objective.TargetProgress);
		Objective.bIsCompleted = Objective.CurrentProgress >= Objective.TargetProgress;

		OnQuestObjectiveUpdated.Broadcast(Objective);
		BroadcastTaskProgress(Task);

		if (Task.State == EExQuestState::Active && Task.IsFullyCompleted())
		{
			Task.State = EExQuestState::Completed;
			BroadcastTaskStateChange(Task);
			BroadcastTaskProgress(Task);
			HandleQuestCompleted(Task);
		}
		else
		{
			SyncRuntimeStateCache();
		}

		return true;
	}

	return false;
}

bool UExQuestManagerSubsystem::UpdateQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveId, int32 NewProgress)
{
	return FindAndUpdateTask(TaskId, [this, &ObjectiveId, NewProgress](FExQuestTask& Task) -> bool
	{
		return ApplyObjectiveProgress(Task, ObjectiveId, NewProgress);
	});
}

bool UExQuestManagerSubsystem::IncrementQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveId, int32 Delta)
{
	FExQuestTask Task;
	if (!CurrentQuestData.FindTaskById(TaskId, Task))
	{
		return false;
	}

	for (const FExQuestObjective& Objective : Task.Objectives)
	{
		if (Objective.ObjectiveId == ObjectiveId)
		{
			return UpdateQuestObjective(TaskId, ObjectiveId, Objective.CurrentProgress + Delta);
		}
	}

	return false;
}

bool UExQuestManagerSubsystem::CompleteQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveId)
{
	FExQuestTask Task;
	if (!CurrentQuestData.FindTaskById(TaskId, Task))
	{
		return false;
	}

	for (const FExQuestObjective& Objective : Task.Objectives)
	{
		if (Objective.ObjectiveId == ObjectiveId)
		{
			return UpdateQuestObjective(TaskId, ObjectiveId, Objective.TargetProgress);
		}
	}

	return false;
}

TArray<FExQuestTask> UExQuestManagerSubsystem::GetActiveQuests() const
{
	return CurrentQuestData.GetAllActiveTasks();
}

TArray<FExQuestTask> UExQuestManagerSubsystem::GetAllQuests() const
{
	return CurrentQuestData.AllTasks;
}

TArray<FExQuestTask> UExQuestManagerSubsystem::GetRootQuests() const
{
	return CurrentQuestData.GetRootTasks();
}

TArray<FExQuestTask> UExQuestManagerSubsystem::GetSubQuests(const FGameplayTag& ParentTaskId) const
{
	return CurrentQuestData.GetSubTasks(ParentTaskId);
}

bool UExQuestManagerSubsystem::GetQuestById(const FGameplayTag& TaskId, FExQuestTask& OutTask) const
{
	return CurrentQuestData.FindTaskById(TaskId, OutTask);
}

void UExQuestManagerSubsystem::ResetAllQuests()
{
	for (FExQuestTask& Task : CurrentQuestData.AllTasks)
	{
		if (const EExQuestState* InitialState = InitialTaskStates.Find(Task.TaskId))
		{
			Task.State = *InitialState;
		}
		else
		{
			Task.State = EExQuestState::Locked;
		}

		ResetTaskObjectives(Task);
		BroadcastTaskStateChange(Task);
		BroadcastTaskProgress(Task);
	}

	NotifyQuestDataRefreshed();
}

FString UExQuestManagerSubsystem::SaveQuestProgress() const
{
	return SaveQuestProgressAsJson();
}

FString UExQuestManagerSubsystem::SaveQuestProgressAsJson() const
{
	return FExQuestSaveHelper::SerializeProgressToJson(CurrentQuestData);
}

bool UExQuestManagerSubsystem::LoadQuestProgressFromJson(const FString& JsonSaveData)
{
	if (!FExQuestSaveHelper::DeserializeProgressFromJson(JsonSaveData, CurrentQuestData))
	{
		return false;
	}

	CurrentQuestData.RebuildIndices();
	NotifyQuestDataRefreshed();
	return true;
}

FString UExQuestManagerSubsystem::SaveQuestProgressAsTextV1() const
{
	FString SaveString = FString(ExQuestSaveFormat::TextV1Header) + TEXT("\n");
	for (const FExQuestTask& Task : CurrentQuestData.AllTasks)
	{
		SaveString += FString::Printf(TEXT("%s|%d\n"), *Task.TaskId.ToString(), static_cast<int32>(Task.State));
		for (const FExQuestObjective& Objective : Task.Objectives)
		{
			SaveString += FString::Printf(
				TEXT("  %s|%d|%d\n"),
				*Objective.ObjectiveId.ToString(),
				Objective.CurrentProgress,
				Objective.bIsCompleted ? 1 : 0);
		}
	}
	return SaveString;
}

bool UExQuestManagerSubsystem::LoadQuestProgress(const FString& SaveData)
{
	if (SaveData.IsEmpty())
	{
		return false;
	}

	if (FExQuestSaveHelper::IsJsonSaveFormat(SaveData))
	{
		return LoadQuestProgressFromJson(SaveData);
	}

	return LoadQuestProgressLegacyText(SaveData);
}

bool UExQuestManagerSubsystem::LoadQuestProgressLegacyText(const FString& SaveData)
{
	TArray<FString> Lines;
	SaveData.ParseIntoArray(Lines, TEXT("\n"), true);

	bool bParsedAny = false;
	FGameplayTag CurrentTaskId;

	for (const FString& Line : Lines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();
		if (TrimmedLine.IsEmpty() || TrimmedLine.StartsWith(TEXT("#")))
		{
			continue;
		}

		if (!TrimmedLine.StartsWith(TEXT("  ")))
		{
			TArray<FString> Parts;
			TrimmedLine.ParseIntoArray(Parts, TEXT("|"), true);
			if (Parts.Num() < 2)
			{
				continue;
			}

			CurrentTaskId = FGameplayTag::RequestGameplayTag(FName(*Parts[0]), false);
			if (!CurrentTaskId.IsValid())
			{
				continue;
			}

			const int32 StateValue = FCString::Atoi(*Parts[1]);
			if (StateValue < static_cast<int32>(EExQuestState::Inactive) || StateValue > static_cast<int32>(EExQuestState::Locked))
			{
				continue;
			}

			const EExQuestState State = static_cast<EExQuestState>(StateValue);
			const bool bUpdated = FindAndUpdateTask(CurrentTaskId, [State](FExQuestTask& Task) -> bool
			{
				Task.State = State;
				return true;
			});

			if (bUpdated)
			{
				bParsedAny = true;
			}
		}
		else if (CurrentTaskId.IsValid())
		{
			const FString ObjectiveLine = TrimmedLine.Mid(2).TrimStartAndEnd();
			TArray<FString> Parts;
			ObjectiveLine.ParseIntoArray(Parts, TEXT("|"), true);
			if (Parts.Num() < 3)
			{
				continue;
			}

			const FGameplayTag ObjectiveId = FGameplayTag::RequestGameplayTag(FName(*Parts[0]), false);
			if (!ObjectiveId.IsValid())
			{
				continue;
			}

			const int32 Progress = FCString::Atoi(*Parts[1]);
			const bool bCompleted = FCString::Atoi(*Parts[2]) != 0;

			const bool bUpdated = FindAndUpdateTask(CurrentTaskId, [ObjectiveId, Progress, bCompleted](FExQuestTask& Task) -> bool
			{
				for (FExQuestObjective& Objective : Task.Objectives)
				{
					if (Objective.ObjectiveId == ObjectiveId)
					{
						Objective.CurrentProgress = Progress;
						Objective.bIsCompleted = bCompleted;
						return true;
					}
				}
				return false;
			});

			if (bUpdated)
			{
				bParsedAny = true;
			}
		}
	}

	if (bParsedAny)
	{
		CurrentQuestData.RebuildIndices();
		NotifyQuestDataRefreshed();
	}

	return bParsedAny;
}

bool UExQuestManagerSubsystem::FindAndUpdateTask(const FGameplayTag& TaskId, TFunctionRef<bool(FExQuestTask&)> UpdateFunc)
{
	FExQuestTask* Task = nullptr;
	if (!CurrentQuestData.FindMutableTaskById(TaskId, Task) || Task == nullptr)
	{
		return false;
	}

	return UpdateFunc(*Task);
}
