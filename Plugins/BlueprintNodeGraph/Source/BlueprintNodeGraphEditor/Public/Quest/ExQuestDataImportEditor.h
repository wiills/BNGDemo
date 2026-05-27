// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UDataTable;
class UExQuestDataAsset;

/** Quest-specific forwarding wrappers around FExDataTableImportEditor. */
class FExQuestDataImportEditor
{
public:
	static bool ImportTableToPairedDataAsset(
		UDataTable* TaskTable,
		bool bSavePackages,
		FString& OutMessage,
		bool bPromptOnSave = true);

	static UExQuestDataAsset* FindPairedDataAsset(const UDataTable* TaskTable);

	static UExQuestDataAsset* FindOrCreatePairedDataAsset(const UDataTable* TaskTable, bool& bOutCreatedNew);

	static void RegisterContentBrowserMenus();
	static void UnregisterContentBrowserMenus();

	static void RegisterAutoImportOnSave();
	static void UnregisterAutoImportOnSave();
};
