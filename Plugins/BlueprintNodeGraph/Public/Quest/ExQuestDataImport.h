// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UDataTable;
class UExQuestDataAsset;
struct FExQuestTaskDefinition;

/** Result of importing rows from a quest task DataTable. */
struct BLUEPRINTNODEGRAPH_API FExQuestDataImportResult
{
	int32 ImportedTaskCount = 0;
	int32 SkippedRowCount = 0;
	bool bSuccess = false;
	FString Message;
};

/** Shared import helpers (runtime-safe; editor uses for DT → DA). */
class BLUEPRINTNODEGRAPH_API FExQuestDataImportUtil
{
public:
	/** DT_Quest_TestMap → DA_Quest_TestMap */
	static FString GetDataAssetNameForTableAssetName(const FString& TableAssetName);

	static FString GetDataAssetNameForTable(const UDataTable* TaskTable);

	static bool IsCompatibleQuestTaskTable(const UDataTable* TaskTable);

	static bool GatherTaskDefinitionsFromTable(
		const UDataTable* TaskTable,
		TArray<FExQuestTaskDefinition>& OutDefinitions,
		int32& OutSkippedRows);

	static FExQuestDataImportResult ApplyDefinitionsToDataAsset(
		UExQuestDataAsset* QuestAsset,
		const TArray<FExQuestTaskDefinition>& Definitions,
		const UDataTable* SourceTable);
};
