// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestManagerSubsystem.h"

void UExQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UExQuestManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UExQuestManagerSubsystem::LoadQuestData(const FExQuestData& QuestData)
{
	CurrentQuestData = QuestData;
}

bool UExQuestManagerSubsystem::ActivateQuest(const FGameplayTag& TaskId)
{
	return FindAndUpdateTask(CurrentQuestData.AllTasks, TaskId, [this](FExQuestTask& Task) -> bool
	{
		if (Task.CanActivate())
		{
			Task.State = EExQuestState::Active;
			OnQuestStateChanged.Broadcast(Task);
			OnQuestProgressChanged.Broadcast(Task.TaskId, Task.GetCompletionPercent());
			return true;
		}
		return false;
	});
}

bool UExQuestManagerSubsystem::CompleteQuest(const FGameplayTag& TaskId)
{
	return FindAndUpdateTask(CurrentQuestData.AllTasks, TaskId, [this](FExQuestTask& Task) -> bool
	{
		if (Task.State == EExQuestState::Active)
		{
			Task.State = EExQuestState::Completed;
			OnQuestStateChanged.Broadcast(Task);
			OnQuestProgressChanged.Broadcast(Task.TaskId, 100.0f);
			return true;
		}
		return false;
	});
}

bool UExQuestManagerSubsystem::FailQuest(const FGameplayTag& TaskId)
{
	return FindAndUpdateTask(CurrentQuestData.AllTasks, TaskId, [this](FExQuestTask& Task) -> bool
	{
		if (Task.State == EExQuestState::Active)
		{
			Task.State = EExQuestState::Failed;
			OnQuestStateChanged.Broadcast(Task);
			return true;
		}
		return false;
	});
}

bool UExQuestManagerSubsystem::UpdateQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveId, int32 NewProgress)
{
	return FindAndUpdateTask(CurrentQuestData.AllTasks, TaskId, [this, &ObjectiveId, NewProgress](FExQuestTask& Task) -> bool
	{
		for (FExQuestObjective& Objective : Task.Objectives)
		{
			if (Objective.ObjectiveId == ObjectiveId)
			{
				Objective.CurrentProgress = FMath::Clamp(NewProgress, 0, Objective.TargetProgress);
				if (Objective.CurrentProgress >= Objective.TargetProgress)
				{
					Objective.bIsCompleted = true;
				}

				OnQuestObjectiveUpdated.Broadcast(Objective);
				OnQuestProgressChanged.Broadcast(Task.TaskId, Task.GetCompletionPercent());

				if (Task.IsFullyCompleted())
				{
					Task.State = EExQuestState::Completed;
					OnQuestStateChanged.Broadcast(Task);
				}

				return true;
			}
		}
		return false;
	});
}

bool UExQuestManagerSubsystem::CompleteQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveId)
{
	return FindAndUpdateTask(CurrentQuestData.AllTasks, TaskId, [this, &ObjectiveId](FExQuestTask& Task) -> bool
	{
		for (FExQuestObjective& Objective : Task.Objectives)
		{
			if (Objective.ObjectiveId == ObjectiveId)
			{
				Objective.bIsCompleted = true;
				Objective.CurrentProgress = Objective.TargetProgress;

				OnQuestObjectiveUpdated.Broadcast(Objective);
				OnQuestProgressChanged.Broadcast(Task.TaskId, Task.GetCompletionPercent());

				if (Task.IsFullyCompleted())
				{
					Task.State = EExQuestState::Completed;
					OnQuestStateChanged.Broadcast(Task);
				}

				return true;
			}
		}
		return false;
	});
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
		Task.State = EExQuestState::Locked;
		for (FExQuestObjective& Objective : Task.Objectives)
		{
			Objective.bIsCompleted = false;
			Objective.CurrentProgress = 0;
		}
	}
}

FString UExQuestManagerSubsystem::SaveQuestProgress() const
{
	FString SaveString;
	for (const FExQuestTask& Task : CurrentQuestData.AllTasks)
	{
		SaveString += FString::Printf(TEXT("%s|%d\n"), *Task.TaskId.ToString(), (int32)Task.State);
		for (const FExQuestObjective& Objective : Task.Objectives)
		{
			SaveString += FString::Printf(TEXT("  %s|%d|%d\n"), *Objective.ObjectiveId.ToString(), Objective.CurrentProgress, Objective.bIsCompleted ? 1 : 0);
		}
	}
	return SaveString;
}

bool UExQuestManagerSubsystem::LoadQuestProgress(const FString& SaveData)
{
	TArray<FString> Lines;
	SaveData.ParseIntoArray(Lines, TEXT("\n"), true);

	FGameplayTag CurrentTaskId;
	for (const FString& Line : Lines)
	{
		FString TrimmedLine = Line.TrimStartAndEnd();
		if (TrimmedLine.IsEmpty()) continue;

		if (!TrimmedLine.StartsWith(TEXT("  ")))
		{
			TArray<FString> Parts;
			TrimmedLine.ParseIntoArray(Parts, TEXT("|"), true);
			if (Parts.Num() >= 2)
			{
				CurrentTaskId = FGameplayTag::RequestGameplayTag(FName(*Parts[0]));
				EExQuestState State = (EExQuestState)FCString::Atoi(*Parts[1]);
				FindAndUpdateTask(CurrentQuestData.AllTasks, CurrentTaskId, [State](FExQuestTask& Task) -> bool
				{
					Task.State = State;
					return true;
				});
			}
		}
	}
	return true;
}

bool UExQuestManagerSubsystem::FindAndUpdateTask(TArray<FExQuestTask>& Tasks, const FGameplayTag& TaskId, TFunctionRef<bool(FExQuestTask&)> UpdateFunc)
{
	for (FExQuestTask& Task : Tasks)
	{
		if (Task.TaskId == TaskId)
		{
			return UpdateFunc(Task);
		}
	}
	return false;
}
