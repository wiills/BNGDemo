// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestTypes.h"

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
		if (!Objective.bIsOptional && !Objective.bIsCompleted)
		{
			return false;
		}
	}

	return true;
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
			if (Objective.bIsCompleted)
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

void FExQuestData::RebuildIndices()
{
	TaskIdToIndex.Empty();
	ObjectiveIdToTaskIndex.Empty();
	ParentTaskIdToChildIndices.Empty();
	PreTaskIdToDependentIndices.Empty();

	for (int32 TaskIndex = 0; TaskIndex < AllTasks.Num(); ++TaskIndex)
	{
		const FExQuestTask& Task = AllTasks[TaskIndex];
		if (!Task.TaskId.IsValid())
		{
			continue;
		}

		TaskIdToIndex.Add(Task.TaskId, TaskIndex);

		if (Task.ParentTaskId.IsValid())
		{
			ParentTaskIdToChildIndices.FindOrAdd(Task.ParentTaskId).Add(TaskIndex);
		}

		for (const FGameplayTag& PreTaskId : Task.PreTaskIds)
		{
			PreTaskIdToDependentIndices.FindOrAdd(PreTaskId).Add(TaskIndex);
		}

		for (const FExQuestObjective& Objective : Task.Objectives)
		{
			if (Objective.ObjectiveId.IsValid())
			{
				ObjectiveIdToTaskIndex.Add(Objective.ObjectiveId, TaskIndex);
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

bool FExQuestData::FindTaskIdByObjectiveId(const FGameplayTag& ObjectiveId, FGameplayTag& OutTaskId) const
{
	if (const int32* TaskIndex = ObjectiveIdToTaskIndex.Find(ObjectiveId))
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
			if (Objective.ObjectiveId == ObjectiveId)
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
			ObjRuntime.ObjectiveId = Objective.ObjectiveId;
			ObjRuntime.CurrentProgress = Objective.CurrentProgress;
			ObjRuntime.bIsCompleted = Objective.bIsCompleted;
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
				if (Objective.ObjectiveId == ObjRuntime.ObjectiveId)
				{
					Objective.CurrentProgress = ObjRuntime.CurrentProgress;
					Objective.bIsCompleted = ObjRuntime.bIsCompleted;
					break;
				}
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
