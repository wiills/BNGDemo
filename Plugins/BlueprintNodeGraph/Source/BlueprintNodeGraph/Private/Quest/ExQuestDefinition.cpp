// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestDefinition.h"

#include "Quest/ExQuestDataImport.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"

FExQuestTask FExQuestTaskTableRow::ToRuntimeTask() const
{
	FExQuestTask Task;
	Task.TaskId = TaskId;
	Task.TaskName = TaskName;
	Task.Description = Description;
	Task.ContextID = ContextID;
	Task.State = InitialState;
	Task.InitialState = InitialState;
	Task.SubTaskIds = SubTaskIds;
	Task.PreTaskIds = PreTaskIds;
	Task.NextTaskIds = NextTaskIds;
	Task.bIsRepeatable = bIsRepeatable;
	Task.UIConfig = UIConfig;

	Task.Objectives = Objectives;
	for (FExQuestObjective& Obj : Task.Objectives)
	{
		Obj.State = Obj.InitialState;
		Obj.CurrentProgress = 0;
	}

	return Task;
}

FExQuestData UExQuestDataAsset::BuildInitialQuestData() const
{
	FExQuestData Data;
	Data.QuestSetId = QuestSetId.IsEmpty() ? GetFName().ToString() : QuestSetId;
	Data.QuestSetName = QuestSetName;
	Data.AllTasks.Reserve(TaskDefinitions.Num());

	for (const FExQuestTask& TaskDef : TaskDefinitions)
	{
		if (TaskDef.TaskId.IsValid())
		{
			FExQuestTask Task = TaskDef;
			Task.State = Task.InitialState;
			for (FExQuestObjective& Obj : Task.Objectives)
			{
				Obj.State = Obj.InitialState;
				Obj.CurrentProgress = 0;
			}
			Data.AllTasks.Add(Task);
		}
	}

	Data.RebuildIndices();
	return Data;
}

FExQuestData UExQuestDataAsset::BuildQuestDataFromTaskTable(
	const UDataTable* TaskTable,
	const FText& InQuestSetName,
	const FString& InQuestSetId)
{
	FExQuestData Data;
	if (!TaskTable)
	{
		return Data;
	}

	if (TaskTable->GetRowStruct() != FExQuestTaskTableRow::StaticStruct())
	{
		UE_LOG(LogBlueprintNodeGraph, Warning,
			TEXT("BuildQuestDataFromTaskTable: expected row struct FExQuestTaskTableRow, got '%s'"),
			TaskTable->GetRowStruct() ? *TaskTable->GetRowStruct()->GetName() : TEXT("null"));
		return Data;
	}

	Data.QuestSetName = InQuestSetName;
	Data.QuestSetId = InQuestSetId.IsEmpty() ? TaskTable->GetName() : InQuestSetId;

	TaskTable->ForeachRow<FExQuestTaskTableRow>(TEXT("BuildQuestDataFromTaskTable"),
		[&Data](const FName& RowName, const FExQuestTaskTableRow& Row)
		{
			if (!Row.TaskId.IsValid())
			{
				UE_LOG(LogBlueprintNodeGraph, Warning,
					TEXT("BuildQuestDataFromTaskTable: row '%s' has invalid TaskId, skipped"),
					*RowName.ToString());
				return;
			}

			Data.AllTasks.Add(Row.ToRuntimeTask());
		});

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

void UExQuestDataAsset::SetSourceTaskTable(const UDataTable* TaskTable)
{
	SourceTaskTable = const_cast<UDataTable*>(TaskTable);
}

void UExQuestDataAsset::EditorImportFromSourceTaskTable()
{
	if (!SourceTaskTable)
	{
		UE_LOG(LogBlueprintNodeGraph, Warning, TEXT("EditorImportFromSourceTaskTable: SourceTaskTable is not set"));
		return;
	}

	ImportTaskDefinitionsFromDataTable(SourceTaskTable);
}

FExQuestDataImportResult UExQuestDataAsset::ImportTaskDefinitionsFromDataTable(UDataTable* TaskTable)
{
	FExQuestDataImportResult Result;

	TArray<FExQuestTask> Definitions;
	int32 SkippedRows = 0;
	if (!FExQuestDataImportUtil::GatherTaskDefinitionsFromTable(TaskTable, Definitions, SkippedRows))
	{
		Result.Message = TEXT("Incompatible DataTable (row struct must be FExQuestTaskTableRow)");
		return Result;
	}

	Result = FExQuestDataImportUtil::ApplyDefinitionsToDataAsset(this, Definitions, TaskTable);
	Result.SkippedRowCount = SkippedRows;
	return Result;
}
#endif
