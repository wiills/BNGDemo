// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Quest/ExQuestTypes.h"
#include "ExQuestSave.generated.h"

/** JSON save format version (matches #ExQuestSaveV2 text header) */
static constexpr int32 ExQuestSaveJsonVersion = 2;

USTRUCT()
struct FExQuestObjectiveSaveJson
{
	GENERATED_BODY()

	UPROPERTY()
	FString ObjectiveTag;

	UPROPERTY()
	int32 CurrentProgress = 0;

	UPROPERTY()
	bool bIsCompleted = false;
};

USTRUCT()
struct FExQuestTaskSaveJson
{
	GENERATED_BODY()

	UPROPERTY()
	FString TaskId;

	UPROPERTY()
	int32 State = 0;

	UPROPERTY()
	TArray<FExQuestObjectiveSaveJson> Objectives;
};

USTRUCT()
struct FExQuestProgressSaveJson
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Version = ExQuestSaveJsonVersion;

	UPROPERTY()
	FString QuestSetId;

	UPROPERTY()
	TArray<FExQuestTaskSaveJson> Tasks;
};

class BLUEPRINTNODEGRAPH_API FExQuestSaveHelper
{
public:
	static const TCHAR* JsonSaveHeader;

	static FString SerializeProgressToJson(const FExQuestData& QuestData);
	static bool DeserializeProgressFromJson(const FString& SaveData, FExQuestData& InOutQuestData);
	static bool IsJsonSaveFormat(const FString& SaveData);
};
