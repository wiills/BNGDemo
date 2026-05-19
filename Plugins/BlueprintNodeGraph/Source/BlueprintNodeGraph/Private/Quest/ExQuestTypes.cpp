// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestTypes.h"

bool FExQuestTask::CanActivate() const
{
	if (State != EExQuestState::Locked && State != EExQuestState::Inactive)
	{
		return false;
	}

	return true;
}

bool FExQuestTask::IsFullyCompleted() const
{
	if (State != EExQuestState::Completed)
	{
		return false;
	}

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

	return (float)CompletedItems / (float)TotalItems * 100.0f;
}

bool FExQuestData::FindTaskById(const FGameplayTag& TaskId, FExQuestTask& OutTask) const
{
	return FindTaskInList(AllTasks, TaskId, OutTask);
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
	for (const FExQuestTask& Task : AllTasks)
	{
		if (Task.ParentTaskId == ParentTaskId)
		{
			SubTasks.Add(Task);
		}
	}
	return SubTasks;
}
