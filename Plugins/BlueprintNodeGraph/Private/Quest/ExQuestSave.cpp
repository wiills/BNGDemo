// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestSave.h"
#include "Quest/ExQuestTypes.h"
#include "JsonObjectConverter.h"

const TCHAR* FExQuestSaveHelper::JsonSaveHeader = TEXT("#ExQuestSaveV2");

bool FExQuestSaveHelper::IsJsonSaveFormat(const FString& SaveData)
{
	const FString Trimmed = SaveData.TrimStartAndEnd();
	return Trimmed.StartsWith(JsonSaveHeader) || Trimmed.StartsWith(TEXT("{"));
}

FString FExQuestSaveHelper::SerializeProgressToJson(const FExQuestData& QuestData)
{
	FExQuestProgressSaveJson SaveJson;
	SaveJson.Version = ExQuestSaveJsonVersion;
	SaveJson.QuestSetId = QuestData.QuestSetId;
	SaveJson.Tasks.Reserve(QuestData.AllTasks.Num());

	for (const FExQuestTask& Task : QuestData.AllTasks)
	{
		FExQuestTaskSaveJson TaskJson;
		TaskJson.TaskId = Task.TaskId.ToString();
		TaskJson.State = static_cast<int32>(Task.State);

		for (const FExQuestObjective& Objective : Task.Objectives)
		{
			FExQuestObjectiveSaveJson ObjJson;
			ObjJson.ObjectiveTag = Objective.ObjectiveTag.ToString();
			ObjJson.CurrentProgress = Objective.CurrentProgress;
			ObjJson.bIsCompleted = Objective.bIsCompleted;
			TaskJson.Objectives.Add(ObjJson);
		}

		SaveJson.Tasks.Add(TaskJson);
	}

	FString JsonBody;
	if (!FJsonObjectConverter::UStructToJsonObjectString(SaveJson, JsonBody))
	{
		return FString();
	}

	return FString::Printf(TEXT("%s\n%s"), JsonSaveHeader, *JsonBody);
}

bool FExQuestSaveHelper::DeserializeProgressFromJson(const FString& SaveData, FExQuestData& InOutQuestData)
{
	FString JsonBody = SaveData.TrimStartAndEnd();
	if (JsonBody.StartsWith(JsonSaveHeader))
	{
		JsonBody.RemoveFromStart(JsonSaveHeader);
		JsonBody.TrimStartAndEndInline();
	}

	if (JsonBody.IsEmpty())
	{
		return false;
	}

	FExQuestProgressSaveJson SaveJson;
	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonBody, &SaveJson, 0, 0))
	{
		return false;
	}

	if (!SaveJson.QuestSetId.IsEmpty() && !InOutQuestData.QuestSetId.IsEmpty() && SaveJson.QuestSetId != InOutQuestData.QuestSetId)
	{
		return false;
	}

	bool bAppliedAny = false;

	for (const FExQuestTaskSaveJson& TaskJson : SaveJson.Tasks)
	{
		const FGameplayTag TaskId = FGameplayTag::RequestGameplayTag(FName(*TaskJson.TaskId), false);
		if (!TaskId.IsValid())
		{
			continue;
		}

		FExQuestTask* Task = nullptr;
		if (!InOutQuestData.FindMutableTaskById(TaskId, Task) || Task == nullptr)
		{
			continue;
		}

		if (TaskJson.State >= static_cast<int32>(EExQuestState::Inactive) && TaskJson.State <= static_cast<int32>(EExQuestState::Locked))
		{
			Task->State = static_cast<EExQuestState>(TaskJson.State);
		}

		for (const FExQuestObjectiveSaveJson& ObjJson : TaskJson.Objectives)
		{
			const FGameplayTag ObjectiveTag = FGameplayTag::RequestGameplayTag(FName(*ObjJson.ObjectiveTag), false);
			if (!ObjectiveTag.IsValid())
			{
				continue;
			}

			for (FExQuestObjective& Objective : Task->Objectives)
			{
				if (Objective.ObjectiveTag == ObjectiveTag)
				{
					Objective.CurrentProgress = ObjJson.CurrentProgress;
					Objective.bIsCompleted = ObjJson.bIsCompleted;
					bAppliedAny = true;
					break;
				}
			}
		}

		bAppliedAny = true;
	}

	return bAppliedAny;
}
