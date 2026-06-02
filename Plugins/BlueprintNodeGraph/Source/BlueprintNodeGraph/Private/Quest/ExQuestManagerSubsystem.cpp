// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestManagerSubsystem.h"
#include "Quest/ExQuestDefinition.h"
#include "Quest/ExQuestSave.h"
#include "Quest/ExQuestReplicationComponent.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "BlueprintTool/LatentTasks/ExLatentTask_Quest.h"

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
	DestroyAllLatentTasks();
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
	OnQuestProgressChanged.Broadcast(Task.TaskId, Task.GetAggregateCompletionPercent(CurrentQuestData));
}

void UExQuestManagerSubsystem::NotifyQuestDataRefreshed()
{
	SyncRuntimeStateCache();
	OnQuestDataLoaded.Broadcast();
}

void UExQuestManagerSubsystem::CommitAuthorityReplication()
{
	if (bApplyingReplicatedView)
	{
		return; // NOTE: 客户端应用复制视图时不要回推，避免循环
	}

	SyncRuntimeStateCache();

	if (UExQuestReplicationComponent* Rep = UExQuestReplicationComponent::Get(this))
	{
		if (Rep->IsAuthorityEndpoint())
		{
			Rep->PublishStateFromAuthorityManager(this);
		}
	}
}

void UExQuestManagerSubsystem::ApplyReplicatedQuestView(UExQuestDataAsset* DefinitionAsset, const FExQuestRuntimeState& RuntimeState)
{
	bApplyingReplicatedView = true;

	if (DefinitionAsset)
	{
		const bool bSameAsset = LoadedQuestAsset == DefinitionAsset;
		const bool bSameSetId = !RuntimeState.QuestSetId.IsEmpty() && CurrentQuestData.QuestSetId == RuntimeState.QuestSetId;

		if (!bSameAsset || !bSameSetId)
		{
			FExQuestData NewData = DefinitionAsset->BuildInitialQuestData();
			LoadedQuestAsset = DefinitionAsset;
			CurrentQuestData = MoveTemp(NewData);
			CurrentQuestData.RebuildIndices();
			CaptureInitialStates();
		}
	}

	CurrentQuestData.ApplyRuntimeState(RuntimeState);
	if (DefinitionAsset)
	{
		CurrentQuestData.EnrichMetadataFrom(DefinitionAsset->BuildInitialQuestData());
	}
	CurrentQuestData.RebuildIndices();
	InitializeActiveTaskExecution();
	SyncRuntimeStateCache();
	NotifyQuestDataRefreshed();

	bApplyingReplicatedView = false;
}

void UExQuestManagerSubsystem::LoadQuestData(const FExQuestData& QuestData)
{
	DestroyAllLatentTasks();

	CurrentQuestData = QuestData;
	CurrentQuestData.RebuildIndices();
	LoadedQuestAsset = nullptr;
	CaptureInitialStates();
	InitializeActiveTaskExecution();
	NotifyQuestDataRefreshed();
	CommitAuthorityReplication();

	RebuildActiveLatentTasks();
}

void UExQuestManagerSubsystem::LoadQuestFromAsset(UExQuestDataAsset* QuestAsset, bool bPreserveRuntime)
{
	if (!QuestAsset)
	{
		return;
	}

	DestroyAllLatentTasks();

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
	InitializeActiveTaskExecution();
	NotifyQuestDataRefreshed();
	CommitAuthorityReplication();

	RebuildActiveLatentTasks();
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
	CommitAuthorityReplication();
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
		Objective.State = EExQuestState::Locked;
		Objective.CurrentProgress = 0;
	}
}

void UExQuestManagerSubsystem::ActivateTaskObjectives(FExQuestTask& Task)
{
	// 已有 Active Objective 时不再解锁后续（含非首个已 Active 的配置）
	for (const FExQuestObjective& Objective : Task.Objectives)
	{
		if (Objective.State == EExQuestState::Active)
		{
			return;
		}
	}

	// 串行解锁：仅激活第一个未完成 Objective
	for (FExQuestObjective& Objective : Task.Objectives)
	{
		if (Objective.IsCompleted())
		{
			continue;
		}

		if (Objective.State == EExQuestState::Locked || Objective.State == EExQuestState::Inactive)
		{
			Objective.State = EExQuestState::Active;
			break;
		}
	}
}

void UExQuestManagerSubsystem::StartLatentTasksForActiveTask(const FGameplayTag& TaskId)
{
	FExQuestTask Task;
	if (!CurrentQuestData.FindTaskById(TaskId, Task) || Task.State != EExQuestState::Active)
	{
		return;
	}

	// Task 级 Latent 可选；默认留空，靠 Objective/SubTask 汇总完成
	CreateLatentTaskForQuest(TaskId);

	// 所有 Active Objective 均 Spawn（不限第一个）
	for (const FExQuestObjective& Objective : Task.Objectives)
	{
		if (Objective.State == EExQuestState::Active)
		{
			CreateLatentTaskForObjective(TaskId, Objective.ObjectiveTag);
		}
	}
}

void UExQuestManagerSubsystem::InitializeActiveTaskExecution()
{
	for (FExQuestTask& Task : CurrentQuestData.AllTasks)
	{
		if (Task.State != EExQuestState::Active)
		{
			continue;
		}

		ActivateTaskObjectives(Task);
		StartLatentTasksForActiveTask(Task.TaskId);
	}
}

void UExQuestManagerSubsystem::TryRollUpParentTasks(const FGameplayTag& CompletedTaskId)
{
	FExQuestTask CompletedTask;
	if (!CurrentQuestData.FindTaskById(CompletedTaskId, CompletedTask))
	{
		return;
	}

	if (!CompletedTask.ParentTaskId.IsValid())
	{
		return;
	}

	const FGameplayTag ParentId = CompletedTask.ParentTaskId;

	// 父 Task 无 Objective 时也可仅靠 SubTask 全完而 IsReadyToComplete
	FindAndUpdateTask(ParentId, [this](FExQuestTask& ParentTask) -> bool
	{
		if (ParentTask.State != EExQuestState::Active || ParentTask.bIsRepeatable)
		{
			return false;
		}

		if (!ParentTask.IsReadyToComplete(CurrentQuestData))
		{
			return false;
		}

		ParentTask.State = EExQuestState::Completed;
		BroadcastTaskStateChange(ParentTask);
		BroadcastTaskProgress(ParentTask);
		HandleQuestCompleted(ParentTask);
		return true;
	});

	FExQuestTask ParentSnapshot;
	if (CurrentQuestData.FindTaskById(ParentId, ParentSnapshot) && ParentSnapshot.State == EExQuestState::Active)
	{
		BroadcastTaskProgress(ParentSnapshot);
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

	DestroyLatentTaskForQuest(Task.TaskId);

	UnlockDependentQuests(Task.TaskId);
	TryRollUpParentTasks(Task.TaskId);

	// 流程边：NextTaskIds 扇出；下游已 Active/Completed 时 CanActivate 为 false，静默跳过
	for (const FGameplayTag& NextId : Task.NextTaskIds)
	{
		FindAndUpdateTask(NextId, [this](FExQuestTask& NextTask) -> bool
		{
			if (NextTask.CanUnlock())
			{
				TryUnlockTask(NextTask);
			}

			if (NextTask.CanActivate() && NextTask.ArePreTasksSatisfied(CurrentQuestData))
			{
				NextTask.State = EExQuestState::Active;
				ActivateTaskObjectives(NextTask);
				BroadcastTaskStateChange(NextTask);
				BroadcastTaskProgress(NextTask);
				StartLatentTasksForActiveTask(NextTask.TaskId);
			}

			return true;
		});
	}

	// 层级边：若刚完成的是父 Task，尝试串行开下一个 Locked/Inactive 子 Task
	ActivateNextSubTask(Task.TaskId);

	// 若刚完成的是子 Task，父仍 Active 时尝试开下一个兄弟 SubTask
	if (Task.ParentTaskId.IsValid())
	{
		FExQuestTask ParentTask;
		if (CurrentQuestData.FindTaskById(Task.ParentTaskId, ParentTask) && ParentTask.State == EExQuestState::Active)
		{
			ActivateNextSubTask(Task.ParentTaskId);
		}
	}

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
		CommitAuthorityReplication();
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
		ActivateTaskObjectives(Task);
		BroadcastTaskStateChange(Task);
		BroadcastTaskProgress(Task);
		return true;
	});

	if (bResult)
	{
		StartLatentTasksForActiveTask(TaskId);
		ActivateNextSubTask(TaskId);
		CommitAuthorityReplication();
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
	if (bCompleted)
	{
		CommitAuthorityReplication();
	}
	return bCompleted;
}

bool UExQuestManagerSubsystem::ForceCompleteQuest(const FGameplayTag& TaskId)
{
	if (!TaskId.IsValid())
	{
		return false;
	}

	bool bCompleted = false;
	FindAndUpdateTask(TaskId, [this, &bCompleted](FExQuestTask& Task) -> bool
	{
		if (Task.State == EExQuestState::Completed)
		{
			return false; // 已完成的 Task 幂等跳过
		}

		// NOTE: 不校验 Active；不自动补全 Objective 进度（测试跳关用）
		Task.State = EExQuestState::Completed;
		BroadcastTaskStateChange(Task);
		BroadcastTaskProgress(Task);
		HandleQuestCompleted(Task);
		bCompleted = true;
		return true;
	});

	if (bCompleted)
	{
		CommitAuthorityReplication();
	}

	return bCompleted;
}

bool UExQuestManagerSubsystem::FailQuest(const FGameplayTag& TaskId)
{
	bool bFailed = false;
	FindAndUpdateTask(TaskId, [this, &bFailed](FExQuestTask& Task) -> bool
	{
		if (Task.State == EExQuestState::Active)
		{
			Task.State = EExQuestState::Failed;
			BroadcastTaskStateChange(Task);
			bFailed = true;
			return true;
		}
		return false;
	});

	if (bFailed)
	{
		DestroyLatentTaskForQuest(TaskId);
		CommitAuthorityReplication();
	}
	return bFailed;
}

bool UExQuestManagerSubsystem::ApplyObjectiveProgress(FExQuestTask& Task, const FGameplayTag& ObjectiveTag, int32 NewProgress)
{
	for (FExQuestObjective& Objective : Task.Objectives)
	{
		if (Objective.ObjectiveTag != ObjectiveTag)
		{
			continue;
		}

		if (Objective.State != EExQuestState::Active)
		{
			return false;
		}

		const bool bWasCompleted = Objective.State == EExQuestState::Completed;
		Objective.CurrentProgress = FMath::Clamp(NewProgress, 0, Objective.TargetProgress);
		Objective.ApplyProgressToState();

		OnQuestObjectiveUpdated.Broadcast(Objective);

		if (Objective.State == EExQuestState::Completed && !bWasCompleted)
		{
			DestroyLatentTaskForObjective(ObjectiveTag);

			// 串行激活下一个 Locked/Inactive Objective
			bool bFoundCurrent = false;
			for (FExQuestObjective& NextObj : Task.Objectives)
			{
				if (!bFoundCurrent)
				{
					if (NextObj.ObjectiveTag == ObjectiveTag)
					{
						bFoundCurrent = true;
					}
					continue;
				}

				if (!NextObj.IsCompleted() && (NextObj.State == EExQuestState::Locked || NextObj.State == EExQuestState::Inactive))
				{
					NextObj.State = EExQuestState::Active;
					OnQuestObjectiveUpdated.Broadcast(NextObj);
					StartLatentTasksForActiveTask(Task.TaskId);
					break;
				}
			}
		}

		BroadcastTaskProgress(Task);

		// Objective 全完且 SubTask 全完 → 自动 Complete 并进入 HandleQuestCompleted 链
		if (Task.State == EExQuestState::Active && Task.IsReadyToComplete(CurrentQuestData))
		{
			Task.State = EExQuestState::Completed;
			BroadcastTaskStateChange(Task);
			BroadcastTaskProgress(Task);
			HandleQuestCompleted(Task);
			CommitAuthorityReplication();
		}
		else
		{
			BroadcastTaskProgress(Task);
			CommitAuthorityReplication();
		}

		return true;
	}

	return false;
}

bool UExQuestManagerSubsystem::UpdateQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag, int32 NewProgress)
{
	const bool bResult = FindAndUpdateTask(TaskId, [this, &ObjectiveTag, NewProgress](FExQuestTask& Task) -> bool
	{
		return ApplyObjectiveProgress(Task, ObjectiveTag, NewProgress);
	});
	return bResult;
}

bool UExQuestManagerSubsystem::IncrementQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag, int32 Delta)
{
	FExQuestTask Task;
	if (!CurrentQuestData.FindTaskById(TaskId, Task))
	{
		return false;
	}

	for (const FExQuestObjective& Objective : Task.Objectives)
	{
		if (Objective.ObjectiveTag == ObjectiveTag)
		{
			return UpdateQuestObjective(TaskId, ObjectiveTag, Objective.CurrentProgress + Delta);
		}
	}

	return false;
}

bool UExQuestManagerSubsystem::CompleteQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag)
{
	FExQuestTask Task;
	if (!CurrentQuestData.FindTaskById(TaskId, Task))
	{
		return false;
	}

	for (const FExQuestObjective& Objective : Task.Objectives)
	{
		if (Objective.ObjectiveTag == ObjectiveTag)
		{
			return UpdateQuestObjective(TaskId, ObjectiveTag, Objective.TargetProgress);
		}
	}

	return false;
}

bool UExQuestManagerSubsystem::NotifyObjectiveProgressByTag(const FGameplayTag& ObjectiveTag, int32 Delta)
{
	if (!ObjectiveTag.IsValid() || Delta <= 0)
	{
		return false;
	}

	FGameplayTag TaskId;
	if (!CurrentQuestData.FindTaskIdByObjectiveTag(ObjectiveTag, TaskId))
	{
		UE_LOG(LogBlueprintNodeGraph, Warning,
			TEXT("NotifyObjectiveProgressByTag: unknown ObjectiveTag '%s'"),
			*ObjectiveTag.ToString());
		return false;
	}

	FExQuestTask Task;
	if (!CurrentQuestData.FindTaskById(TaskId, Task))
	{
		return false;
	}

	if (Task.State != EExQuestState::Active)
	{
		UE_LOG(LogBlueprintNodeGraph, Verbose,
			TEXT("NotifyObjectiveProgressByTag: task '%s' is not Active, ignoring tag '%s'"),
			*TaskId.ToString(),
			*ObjectiveTag.ToString());
		return false;
	}

	return IncrementQuestObjective(TaskId, ObjectiveTag, Delta);
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
	DestroyAllLatentTasks();

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

	InitializeActiveTaskExecution();
	NotifyQuestDataRefreshed();
	CommitAuthorityReplication();
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

	if (LoadedQuestAsset)
	{
		CurrentQuestData.EnrichMetadataFrom(LoadedQuestAsset->BuildInitialQuestData());
	}

	CurrentQuestData.RebuildIndices();
	InitializeActiveTaskExecution();
	NotifyQuestDataRefreshed();
	CommitAuthorityReplication();

	RebuildActiveLatentTasks();
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
				TEXT("  %s|%d|%d|%d\n"),
				*Objective.ObjectiveTag.ToString(),
				Objective.CurrentProgress,
				Objective.IsCompleted() ? 1 : 0,
				static_cast<int32>(Objective.State));
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

			const FGameplayTag ObjectiveTag = FGameplayTag::RequestGameplayTag(FName(*Parts[0]), false);
			if (!ObjectiveTag.IsValid())
			{
				continue;
			}

			const int32 Progress = FCString::Atoi(*Parts[1]);
			const bool bCompleted = FCString::Atoi(*Parts[2]) != 0;
			const int32 StateValue = Parts.Num() >= 4
				? FCString::Atoi(*Parts[3])
				: (bCompleted ? static_cast<int32>(EExQuestState::Completed) : static_cast<int32>(EExQuestState::Locked));

			const bool bUpdated = FindAndUpdateTask(CurrentTaskId, [ObjectiveTag, Progress, bCompleted, StateValue](FExQuestTask& Task) -> bool
			{
				for (FExQuestObjective& Objective : Task.Objectives)
				{
					if (Objective.ObjectiveTag == ObjectiveTag)
					{
						Objective.CurrentProgress = Progress;
						if (StateValue >= static_cast<int32>(EExQuestState::Inactive)
							&& StateValue <= static_cast<int32>(EExQuestState::Locked))
						{
							Objective.State = static_cast<EExQuestState>(StateValue);
						}
						else if (bCompleted)
						{
							Objective.State = EExQuestState::Completed;
						}
						else
						{
							Objective.State = EExQuestState::Locked;
						}
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
		InitializeActiveTaskExecution();
		NotifyQuestDataRefreshed();
		CommitAuthorityReplication();

		RebuildActiveLatentTasks();
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

void UExQuestManagerSubsystem::CreateLatentTaskForQuest(const FGameplayTag& TaskId)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return; // NOTE: LatentTask 仅权威端 Spawn；Client 只看复制结果
	}

	FExQuestTask Task;
	if (!CurrentQuestData.FindTaskById(TaskId, Task))
	{
		return;
	}

	if (!Task.LatentTaskClass)
	{
		return;
	}

	if (ActiveLatentTasks.Contains(TaskId))
	{
		return;
	}

	UExLatentTask_Quest* LatentTask = UExLatentTask_Quest::CreateQuestProxy(World, Task.LatentTaskClass);
	if (!LatentTask)
	{
		UE_LOG(LogBlueprintNodeGraph, Warning,
			TEXT("CreateLatentTaskForQuest: failed to create latent task for '%s'"),
			*TaskId.ToString());
		return;
	}

	LatentTask->QuestTag = TaskId;

	ActiveLatentTasks.Add(TaskId, LatentTask);
	LatentTask->Activate();
}

void UExQuestManagerSubsystem::DestroyLatentTaskForQuest(const FGameplayTag& TaskId)
{
	TObjectPtr<UExLatentTask_Quest> LatentTask;
	if (ActiveLatentTasks.RemoveAndCopyValue(TaskId, LatentTask) && LatentTask)
	{
		LatentTask->Terminate();
	}

	DestroyObjectiveLatentTasksForQuest(TaskId);
}

void UExQuestManagerSubsystem::DestroyAllLatentTasks()
{
	for (const TPair<FGameplayTag, TObjectPtr<UExLatentTask_Quest>>& Pair : ActiveLatentTasks)
	{
		if (Pair.Value)
		{
			Pair.Value->Terminate();
		}
	}

	ActiveLatentTasks.Empty();

	for (const TPair<FGameplayTag, TObjectPtr<UExLatentTask_Quest>>& Pair : ActiveObjectiveLatentTasks)
	{
		if (Pair.Value)
		{
			Pair.Value->Terminate();
		}
	}

	ActiveObjectiveLatentTasks.Empty();
}

void UExQuestManagerSubsystem::RebuildActiveLatentTasks()
{
	DestroyAllLatentTasks();

	for (FExQuestTask& Task : CurrentQuestData.AllTasks)
	{
		if (Task.State != EExQuestState::Active)
		{
			continue;
		}

		bool bHasActiveObjective = false;
		for (const FExQuestObjective& Objective : Task.Objectives)
		{
			if (Objective.State == EExQuestState::Active)
			{
				bHasActiveObjective = true;
				break;
			}
		}

		if (!bHasActiveObjective)
		{
			ActivateTaskObjectives(Task);
		}

		StartLatentTasksForActiveTask(Task.TaskId);
	}
}

void UExQuestManagerSubsystem::CreateLatentTaskForObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}

	FExQuestTask Task;
	if (!CurrentQuestData.FindTaskById(TaskId, Task))
	{
		return;
	}

	const FExQuestObjective* Objective = nullptr;
	for (const FExQuestObjective& Obj : Task.Objectives)
	{
		if (Obj.ObjectiveTag == ObjectiveTag)
		{
			Objective = &Obj;
			break;
		}
	}

	if (!Objective || !Objective->LatentTaskClass)
	{
		return;
	}

	if (ActiveObjectiveLatentTasks.Contains(ObjectiveTag))
	{
		return;
	}

	UExLatentTask_Quest* LatentTask = UExLatentTask_Quest::CreateQuestProxy(World, Objective->LatentTaskClass);
	if (!LatentTask)
	{
		UE_LOG(LogBlueprintNodeGraph, Warning,
			TEXT("CreateLatentTaskForObjective: failed to create latent task for objective '%s'"),
			*ObjectiveTag.ToString());
		return;
	}

	LatentTask->QuestTag = TaskId;
	LatentTask->ObjectiveTag = ObjectiveTag;

	ActiveObjectiveLatentTasks.Add(ObjectiveTag, LatentTask);
	LatentTask->Activate();
}

void UExQuestManagerSubsystem::DestroyLatentTaskForObjective(const FGameplayTag& ObjectiveTag)
{
	TObjectPtr<UExLatentTask_Quest> LatentTask;
	if (!ActiveObjectiveLatentTasks.RemoveAndCopyValue(ObjectiveTag, LatentTask) || !LatentTask)
	{
		return;
	}

	LatentTask->Terminate();
}

void UExQuestManagerSubsystem::DestroyObjectiveLatentTasksForQuest(const FGameplayTag& TaskId)
{
	FExQuestTask Task;
	if (!CurrentQuestData.FindTaskById(TaskId, Task))
	{
		return;
	}

	for (const FExQuestObjective& Objective : Task.Objectives)
	{
		DestroyLatentTaskForObjective(Objective.ObjectiveTag);
	}
}

void UExQuestManagerSubsystem::ActivateNextSubTask(const FGameplayTag& ParentTaskId)
{
	const TArray<FExQuestTask> SubTasks = CurrentQuestData.GetSubTasks(ParentTaskId);

	for (const FExQuestTask& SubTask : SubTasks)
	{
		// 并行 InitialState=Active 或已完成的兄弟 SubTask 跳过
		if (SubTask.State != EExQuestState::Locked && SubTask.State != EExQuestState::Inactive)
		{
			continue;
		}

		if (!SubTask.ArePreTasksSatisfied(CurrentQuestData))
		{
			continue;
		}

		FindAndUpdateTask(SubTask.TaskId, [this](FExQuestTask& OutTask) -> bool
		{
			if (OutTask.State == EExQuestState::Locked)
			{
				TryUnlockTask(OutTask);
			}

			if (OutTask.CanActivate() && OutTask.ArePreTasksSatisfied(CurrentQuestData))
			{
				OutTask.State = EExQuestState::Active;
				ActivateTaskObjectives(OutTask);
				BroadcastTaskStateChange(OutTask);
				BroadcastTaskProgress(OutTask);
				StartLatentTasksForActiveTask(OutTask.TaskId);
			}

			return true;
		});

		return; // NOTE: 只处理第一个待激活子 Task，保持 SubTask 串行语义
	}
}
