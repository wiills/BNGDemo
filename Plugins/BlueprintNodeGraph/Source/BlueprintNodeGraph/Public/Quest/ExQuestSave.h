// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExQuestTypes.h"
#include "ExQuestSave.generated.h"

/** JSON 存档格式版本（对应 #ExQuestSaveV2 文本头） */
static constexpr int32 ExQuestSaveJsonVersion = 2;

USTRUCT()
struct FExQuestObjectiveSaveJson
{
	GENERATED_BODY()

	UPROPERTY()
	FString ObjectiveId;

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

/**
 * @class UExQuestSaveHelper
 * @brief 任务进度 JSON 序列化（FJsonObjectConverter）
 */
class BLUEPRINTNODEGRAPH_API FExQuestSaveHelper
{
public:
	static const TCHAR* JsonSaveHeader;

	static FString SerializeProgressToJson(const FExQuestData& QuestData);
	static bool DeserializeProgressFromJson(const FString& SaveData, FExQuestData& InOutQuestData);
	static bool IsJsonSaveFormat(const FString& SaveData);
};
