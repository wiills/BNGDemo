// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestDefinition.h"

FExQuestTask FExQuestTaskDefinition::ToRuntimeTask() const
{
	FExQuestTask Task;
	Task.TaskId = TaskId;
	Task.TaskName = TaskName;
	Task.Description = Description;
	Task.State = InitialState;
	Task.SubTaskIds = SubTaskIds;
	Task.PreTaskIds = PreTaskIds;
	Task.ParentTaskId = ParentTaskId;
	Task.bIsRepeatable = bIsRepeatable;

	for (const FExQuestObjectiveDefinition& ObjDef : Objectives)
	{
		FExQuestObjective Objective;
		Objective.ObjectiveId = ObjDef.ObjectiveId;
		Objective.Description = ObjDef.Description;
		Objective.TargetProgress = ObjDef.TargetProgress;
		Objective.bIsOptional = ObjDef.bIsOptional;
		Objective.CurrentProgress = 0;
		Objective.bIsCompleted = false;
		Task.Objectives.Add(Objective);
	}

	return Task;
}

FExQuestData UExQuestDataAsset::BuildInitialQuestData() const
{
	FExQuestData Data;
	Data.QuestSetId = QuestSetId.IsEmpty() ? GetFName().ToString() : QuestSetId;
	Data.QuestSetName = QuestSetName;
	Data.AllTasks.Reserve(TaskDefinitions.Num());

	for (const FExQuestTaskDefinition& TaskDef : TaskDefinitions)
	{
		if (TaskDef.TaskId.IsValid())
		{
			Data.AllTasks.Add(TaskDef.ToRuntimeTask());
		}
	}

	Data.RebuildIndices();
	return Data;
}

#if WITH_EDITOR
void UExQuestDataAsset::PostLoad()
{
	Super::PostLoad();

	if (QuestSetId.IsEmpty())
	{
		QuestSetId = GetFName().ToString();
	}
}
#endif
