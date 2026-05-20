// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UDataTable;
class UExQuestDataAsset;

/** Editor-only: DT_Quest_* → DA_Quest_* import and asset creation. */
class FExQuestDataImportEditor
{
public:
	/** Import table rows into paired DA (same folder). Creates DA if missing. */
	static bool ImportTableToPairedDataAsset(
		UDataTable* TaskTable,
		bool bSavePackages,
		FString& OutMessage,
		bool bPromptOnSave = true);

	static UExQuestDataAsset* FindPairedDataAsset(const UDataTable* TaskTable);

	static UExQuestDataAsset* FindOrCreatePairedDataAsset(const UDataTable* TaskTable, bool& bOutCreatedNew);

	static void RegisterContentBrowserMenus();
	static void UnregisterContentBrowserMenus();

	/** When enabled in DefaultBlueprintNodeGraph.ini, sync paired DA after quest task tables are saved. */
	static void RegisterAutoImportOnSave();
	static void UnregisterAutoImportOnSave();
};
