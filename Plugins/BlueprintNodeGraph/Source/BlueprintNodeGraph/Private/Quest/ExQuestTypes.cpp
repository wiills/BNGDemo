// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestTypes.h"

#include "BlueprintTool/Common/ExLatentProxyDefine.h"

void FExQuestObjective::ApplyProgressToState()
{
	if (CurrentProgress >= TargetProgress)
	{
		State = EExQuestState::Completed;
	}
	else if (State == EExQuestState::Completed)
	{
		State = EExQuestState::Active;
	}
}

bool FExQuestTask::CanActivate() const
{
	return State == EExQuestState::Inactive;
}

bool FExQuestTask::CanUnlock() const
{
	return State == EExQuestState::Locked;
}

bool FExQuestTask::ArePreTasksSatisfied(const FExQuestData& QuestData) const
{
	if (PreTaskIds.IsEmpty())
	{
		return true;
	}

	for (const FGameplayTag& PreTaskId : PreTaskIds)
	{
		FExQuestTask PreTask;
		if (!QuestData.FindTaskById(PreTaskId, PreTask) || PreTask.State != EExQuestState::Completed)
		{
			return false;
		}
	}

	return true;
}

bool FExQuestTask::IsFullyCompleted() const
{
	for (const FExQuestObjective& Objective : Objectives)
	{
		if (!Objective.bIsOptional && !Objective.IsCompleted())
		{
			return false;
		}
	}

	return true;
}

bool FExQuestTask::AreAllSubTasksCompleted(const FExQuestData& QuestData) const
{
	if (SubTaskIds.IsEmpty())
	{
		return true;
	}

	for (const FGameplayTag& SubTaskId : SubTaskIds)
	{
		FExQuestTask SubTask;
		if (!QuestData.FindTaskById(SubTaskId, SubTask))
		{
			return false;
		}

		if (SubTask.State != EExQuestState::Completed)
		{
			return false;
		}
	}

	return true;
}

bool FExQuestTask::IsReadyToComplete(const FExQuestData& QuestData) const
{
	// Task 自动 Complete 门槛：必填 Objective 全完 且 SubTaskIds 子 Task 全 Completed
	return IsFullyCompleted() && AreAllSubTasksCompleted(QuestData);
}

float FExQuestTask::GetCompletionPercent() const
{
	if (Objectives.IsEmpty())
	{
		return State == EExQuestState::Completed ? 100.0f : 0.0f;
	}

	int32 TotalItems = 0;
	int32 CompletedItems = 0;

	for (const FExQuestObjective& Objective : Objectives)
	{
		if (!Objective.bIsOptional)
		{
			TotalItems++;
			if (Objective.IsCompleted())
			{
				CompletedItems++;
			}
		}
	}

	if (TotalItems == 0)
	{
		return State == EExQuestState::Completed ? 100.0f : 0.0f;
	}

	return static_cast<float>(CompletedItems) / static_cast<float>(TotalItems) * 100.0f;
}

float FExQuestTask::GetAggregateCompletionPercent(const FExQuestData& QuestData) const
{
	int32 TotalItems = 0;
	int32 CompletedItems = 0;

	for (const FExQuestObjective& Objective : Objectives)
	{
		if (!Objective.bIsOptional)
		{
			TotalItems++;
			if (Objective.IsCompleted())
			{
				CompletedItems++;
			}
		}
	}

	for (const FGameplayTag& SubTaskId : SubTaskIds)
	{
		FExQuestTask SubTask;
		if (!QuestData.FindTaskById(SubTaskId, SubTask))
		{
			continue;
		}

		TotalItems++;
		if (SubTask.State == EExQuestState::Completed)
		{
			CompletedItems++;
		}
	}

	if (TotalItems == 0)
	{
		return State == EExQuestState::Completed ? 100.0f : 0.0f;
	}

	return static_cast<float>(CompletedItems) / static_cast<float>(TotalItems) * 100.0f;
}

void FExQuestData::RebuildIndices()
{
	TaskIdToIndex.Empty();
	ObjectiveTagToTaskIndex.Empty();
	ParentTaskIdToChildIndices.Empty();
	PreTaskIdToDependentIndices.Empty();

	// 第一遍：TaskId → 数组下标
	for (int32 TaskIndex = 0; TaskIndex < AllTasks.Num(); ++TaskIndex)
	{
		const FExQuestTask& Task = AllTasks[TaskIndex];
		if (Task.TaskId.IsValid())
		{
			TaskIdToIndex.Add(Task.TaskId, TaskIndex);
		}
	}

	// 第二遍：任务树 — 父行 SubTaskIds 为权威来源，写回子行 ParentTaskId
	TMap<FGameplayTag, FGameplayTag> ChildToParentFromSubTaskIds;
	for (const FExQuestTask& Task : AllTasks)
	{
		if (!Task.TaskId.IsValid())
		{
			continue;
		}

		for (const FGameplayTag& SubTaskId : Task.SubTaskIds)
		{
			if (!SubTaskId.IsValid())
			{
				continue;
			}

			if (const FGameplayTag* ExistingParentId = ChildToParentFromSubTaskIds.Find(SubTaskId))
			{
				if (*ExistingParentId != Task.TaskId)
				{
					UE_LOG(LogBlueprintNodeGraph, Warning,
						TEXT("RebuildIndices: SubTask '%s' listed under multiple parents '%s' and '%s'; using '%s'"),
						*SubTaskId.ToString(),
						*ExistingParentId->ToString(),
						*Task.TaskId.ToString(),
						*Task.TaskId.ToString());
				}
				continue;
			}

			ChildToParentFromSubTaskIds.Add(SubTaskId, Task.TaskId);
		}
	}

	for (const TPair<FGameplayTag, FGameplayTag>& ChildParentPair : ChildToParentFromSubTaskIds)
	{
		const int32* ChildIndex = TaskIdToIndex.Find(ChildParentPair.Key);
		if (!ChildIndex)
		{
			UE_LOG(LogBlueprintNodeGraph, Warning,
				TEXT("RebuildIndices: SubTaskId '%s' not found in AllTasks (referenced by parent '%s')"),
				*ChildParentPair.Key.ToString(),
				*ChildParentPair.Value.ToString());
			continue;
		}

		FExQuestTask& ChildTask = AllTasks[*ChildIndex];
		if (ChildTask.ParentTaskId.IsValid() && ChildTask.ParentTaskId != ChildParentPair.Value)
		{
			UE_LOG(LogBlueprintNodeGraph, Warning,
				TEXT("RebuildIndices: ParentTaskId on '%s' ('%s') overridden by parent SubTaskIds link to '%s'"),
				*ChildParentPair.Key.ToString(),
				*ChildTask.ParentTaskId.ToString(),
				*ChildParentPair.Value.ToString());
		}

		ChildTask.ParentTaskId = ChildParentPair.Value;
	}

	for (int32 TaskIndex = 0; TaskIndex < AllTasks.Num(); ++TaskIndex)
	{
		FExQuestTask& Task = AllTasks[TaskIndex];
		if (!Task.TaskId.IsValid())
		{
			continue;
		}

		if (Task.ParentTaskId.IsValid())
		{
			ParentTaskIdToChildIndices.FindOrAdd(Task.ParentTaskId).Add(TaskIndex);
		}

		// 收集 NextTaskIds 反向边，供后续推导 PreTaskIds
		for (const FGameplayTag& NextId : Task.NextTaskIds)
		{
			if (NextId.IsValid())
			{
				PreTaskIdToDependentIndices.FindOrAdd(NextId).Add(TaskIndex);
			}
		}

		for (const FExQuestObjective& Objective : Task.Objectives)
		{
			if (!Objective.ObjectiveTag.IsValid())
			{
				continue;
			}

			if (const int32* ExistingTaskIndex = ObjectiveTagToTaskIndex.Find(Objective.ObjectiveTag))
			{
				const FGameplayTag ExistingTaskId = AllTasks.IsValidIndex(*ExistingTaskIndex)
					? AllTasks[*ExistingTaskIndex].TaskId
					: FGameplayTag();
				UE_LOG(LogBlueprintNodeGraph, Warning,
					TEXT("RebuildIndices: duplicate ObjectiveTag '%s' on tasks '%s' and '%s'"),
					*Objective.ObjectiveTag.ToString(),
					*ExistingTaskId.ToString(),
					*Task.TaskId.ToString());
			}

			ObjectiveTagToTaskIndex.Add(Objective.ObjectiveTag, TaskIndex);
		}
	}

	// 第三遍：由 NextTaskIds 反推 PreTaskIds（ActivateQuest 前置校验用）
	for (FExQuestTask& Task : AllTasks)
	{
		if (!Task.TaskId.IsValid())
		{
			continue;
		}

		Task.PreTaskIds.Reset();

		if (const TArray<int32>* DependentIndices = PreTaskIdToDependentIndices.Find(Task.TaskId))
		{
			for (const int32 DependentIndex : *DependentIndices)
			{
				if (AllTasks.IsValidIndex(DependentIndex))
				{
					Task.PreTaskIds.AddTag(AllTasks[DependentIndex].TaskId);
				}
			}
		}
	}
}

int32 FExQuestData::FindTaskIndex(const FGameplayTag& TaskId) const
{
	if (const int32* Index = TaskIdToIndex.Find(TaskId))
	{
		return *Index;
	}

	return INDEX_NONE;
}

bool FExQuestData::FindTaskById(const FGameplayTag& TaskId, FExQuestTask& OutTask) const
{
	const int32 Index = FindTaskIndex(TaskId);
	if (AllTasks.IsValidIndex(Index))
	{
		OutTask = AllTasks[Index];
		return true;
	}

	return FindTaskInList(AllTasks, TaskId, OutTask);
}

bool FExQuestData::FindMutableTaskById(const FGameplayTag& TaskId, FExQuestTask*& OutTask)
{
	const int32 Index = FindTaskIndex(TaskId);
	if (AllTasks.IsValidIndex(Index))
	{
		OutTask = &AllTasks[Index];
		return true;
	}

	for (FExQuestTask& Task : AllTasks)
	{
		if (Task.TaskId == TaskId)
		{
			OutTask = &Task;
			return true;
		}
	}

	OutTask = nullptr;
	return false;
}

bool FExQuestData::FindTaskIdByObjectiveTag(const FGameplayTag& ObjectiveTag, FGameplayTag& OutTaskId) const
{
	if (const int32* TaskIndex = ObjectiveTagToTaskIndex.Find(ObjectiveTag))
	{
		if (AllTasks.IsValidIndex(*TaskIndex))
		{
			OutTaskId = AllTasks[*TaskIndex].TaskId;
			return true;
		}
	}

	for (const FExQuestTask& Task : AllTasks)
	{
		for (const FExQuestObjective& Objective : Task.Objectives)
		{
			if (Objective.ObjectiveTag == ObjectiveTag)
			{
				OutTaskId = Task.TaskId;
				return true;
			}
		}
	}

	return false;
}

FExQuestRuntimeState FExQuestData::ExtractRuntimeState() const
{
	FExQuestRuntimeState Runtime;
	Runtime.QuestSetId = QuestSetId;
	Runtime.TaskStates.Reserve(AllTasks.Num());

	for (const FExQuestTask& Task : AllTasks)
	{
		FExQuestTaskRuntime TaskRuntime;
		TaskRuntime.TaskId = Task.TaskId;
		TaskRuntime.State = Task.State;

		for (const FExQuestObjective& Objective : Task.Objectives)
		{
			FExQuestObjectiveRuntime ObjRuntime;
			ObjRuntime.ObjectiveTag = Objective.ObjectiveTag;
			ObjRuntime.CurrentProgress = Objective.CurrentProgress;
			ObjRuntime.State = Objective.State;
			TaskRuntime.Objectives.Add(ObjRuntime);
		}

		Runtime.TaskStates.Add(TaskRuntime);
	}

	return Runtime;
}

void FExQuestData::ApplyRuntimeState(const FExQuestRuntimeState& RuntimeState)
{
	if (!RuntimeState.QuestSetId.IsEmpty())
	{
		QuestSetId = RuntimeState.QuestSetId;
	}

	for (const FExQuestTaskRuntime& TaskRuntime : RuntimeState.TaskStates)
	{
		FExQuestTask* Task = nullptr;
		if (!FindMutableTaskById(TaskRuntime.TaskId, Task) || Task == nullptr)
		{
			continue;
		}

		Task->State = TaskRuntime.State;

		for (const FExQuestObjectiveRuntime& ObjRuntime : TaskRuntime.Objectives)
		{
			for (FExQuestObjective& Objective : Task->Objectives)
			{
				if (Objective.ObjectiveTag == ObjRuntime.ObjectiveTag)
				{
					Objective.CurrentProgress = ObjRuntime.CurrentProgress;
					Objective.State = ObjRuntime.State;
					break;
				}
			}
		}
	}
}

void FExQuestData::EnrichMetadataFrom(const FExQuestData& DefinitionData)
{
	for (FExQuestTask& Task : AllTasks)
	{
		FExQuestTask DefTask;
		if (!DefinitionData.FindTaskById(Task.TaskId, DefTask))
		{
			continue;
		}

		if (Task.TaskName.IsEmpty() && !DefTask.TaskName.IsEmpty())
		{
			Task.TaskName = DefTask.TaskName;
		}

		if (Task.Description.IsEmpty() && !DefTask.Description.IsEmpty())
		{
			Task.Description = DefTask.Description;
		}

		if (Task.EntryViewClass.IsNull() && !DefTask.EntryViewClass.IsNull())
		{
			Task.EntryViewClass = DefTask.EntryViewClass;
		}

		if (!Task.LatentTaskClass && DefTask.LatentTaskClass)
		{
			Task.LatentTaskClass = DefTask.LatentTaskClass;
		}

		if (DefTask.LatentTaskClass)
		{
			Task.LatentTaskPayload = DefTask.LatentTaskPayload;
		}

		// RuntimeState 只保存进度/状态；UI 样式属于定义侧元数据，所以读档后需要从定义快照补回到每个目标。
		for (FExQuestObjective& Objective : Task.Objectives)
		{
			for (const FExQuestObjective& DefObjective : DefTask.Objectives)
			{
				if (Objective.ObjectiveTag != DefObjective.ObjectiveTag)
				{
					continue;
				}

				if (Objective.Description.IsEmpty() && !DefObjective.Description.IsEmpty())
				{
					Objective.Description = DefObjective.Description;
				}

				if (Objective.TargetProgress <= 0 && DefObjective.TargetProgress > 0)
				{
					Objective.TargetProgress = DefObjective.TargetProgress;
				}

				Objective.bUIVisible = DefObjective.bUIVisible;

				if (Objective.EntryViewClass.IsNull() && !DefObjective.EntryViewClass.IsNull())
				{
					Objective.EntryViewClass = DefObjective.EntryViewClass;
				}

				if (!Objective.LatentTaskClass && DefObjective.LatentTaskClass)
				{
					Objective.LatentTaskClass = DefObjective.LatentTaskClass;
				}

				if (DefObjective.LatentTaskClass)
				{
					Objective.LatentTaskPayload = DefObjective.LatentTaskPayload;
				}

				break;
			}
		}
	}
}

bool FExQuestData::CanActivateTask(const FGameplayTag& TaskId) const
{
	FExQuestTask Task;
	if (!FindTaskById(TaskId, Task))
	{
		return false;
	}

	if (!Task.CanActivate())
	{
		return false;
	}

	return Task.ArePreTasksSatisfied(*this);
}

bool FExQuestData::FindTaskInList(const TArray<FExQuestTask>& Tasks, const FGameplayTag& TaskId, FExQuestTask& OutTask) const
{
	for (const FExQuestTask& Task : Tasks)
	{
		if (Task.TaskId == TaskId)
		{
			OutTask = Task;
			return true;
		}
	}
	return false;
}

TArray<FExQuestTask> FExQuestData::GetAllActiveTasks() const
{
	TArray<FExQuestTask> ActiveTasks;
	for (const FExQuestTask& Task : AllTasks)
	{
		if (Task.State == EExQuestState::Active)
		{
			ActiveTasks.Add(Task);
		}
	}
	return ActiveTasks;
}

TArray<FExQuestTask> FExQuestData::GetAllCompletedTasks() const
{
	TArray<FExQuestTask> CompletedTasks;
	for (const FExQuestTask& Task : AllTasks)
	{
		if (Task.State == EExQuestState::Completed)
		{
			CompletedTasks.Add(Task);
		}
	}
	return CompletedTasks;
}

TArray<FExQuestTask> FExQuestData::GetRootTasks() const
{
	TArray<FExQuestTask> RootTasks;
	for (const FExQuestTask& Task : AllTasks)
	{
		if (!Task.ParentTaskId.IsValid())
		{
			RootTasks.Add(Task);
		}
	}
	return RootTasks;
}

TArray<FExQuestTask> FExQuestData::GetSubTasks(const FGameplayTag& ParentTaskId) const
{
	TArray<FExQuestTask> SubTasks;

	if (const TArray<int32>* ChildIndices = ParentTaskIdToChildIndices.Find(ParentTaskId))
	{
		for (const int32 Index : *ChildIndices)
		{
			if (AllTasks.IsValidIndex(Index))
			{
				SubTasks.Add(AllTasks[Index]);
			}
		}
		return SubTasks;
	}

	for (const FExQuestTask& Task : AllTasks)
	{
		if (Task.ParentTaskId == ParentTaskId)
		{
			SubTasks.Add(Task);
		}
	}
	return SubTasks;
}

TArray<FGameplayTag> FExQuestData::GetTaskIdsWithPreTask(const FGameplayTag& PreTaskId) const
{
	TArray<FGameplayTag> DependentIds;

	if (const TArray<int32>* DependentIndices = PreTaskIdToDependentIndices.Find(PreTaskId))
	{
		for (const int32 Index : *DependentIndices)
		{
			if (AllTasks.IsValidIndex(Index))
			{
				DependentIds.Add(AllTasks[Index].TaskId);
			}
		}
		return DependentIds;
	}

	for (const FExQuestTask& Task : AllTasks)
	{
		if (Task.PreTaskIds.HasTag(PreTaskId))
		{
			DependentIds.Add(Task.TaskId);
		}
	}
	return DependentIds;
}
