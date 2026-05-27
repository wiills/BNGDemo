// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestDataImportEditor.h"

#include "Import/ExDataTableImportEditor.h"
#include "Quest/ExQuestDataTableImportHandler.h"
#include "Quest/ExQuestDefinition.h"

UExQuestDataAsset* FExQuestDataImportEditor::FindPairedDataAsset(const UDataTable* TaskTable)
{
	static const FExQuestDataTableImportHandler QuestHandler;
	if (!TaskTable || !QuestHandler.CanImportTable(TaskTable))
	{
		return nullptr;
	}

	return Cast<UExQuestDataAsset>(FExDataTableImportEditor::FindPairedDataAsset(TaskTable, QuestHandler));
}

UExQuestDataAsset* FExQuestDataImportEditor::FindOrCreatePairedDataAsset(const UDataTable* TaskTable, bool& bOutCreatedNew)
{
	static const FExQuestDataTableImportHandler QuestHandler;
	if (!TaskTable || !QuestHandler.CanImportTable(TaskTable))
	{
		bOutCreatedNew = false;
		return nullptr;
	}

	return Cast<UExQuestDataAsset>(
		FExDataTableImportEditor::FindOrCreatePairedDataAsset(TaskTable, QuestHandler, bOutCreatedNew));
}

bool FExQuestDataImportEditor::ImportTableToPairedDataAsset(
	UDataTable* TaskTable,
	bool bSavePackages,
	FString& OutMessage,
	bool bPromptOnSave)
{
	return FExDataTableImportEditor::ImportTableToPairedDataAsset(TaskTable, bSavePackages, OutMessage, bPromptOnSave);
}

void FExQuestDataImportEditor::RegisterContentBrowserMenus()
{
	FExDataTableImportEditor::RegisterContentBrowserMenus();
}

void FExQuestDataImportEditor::UnregisterContentBrowserMenus()
{
	FExDataTableImportEditor::UnregisterContentBrowserMenus();
}

void FExQuestDataImportEditor::RegisterAutoImportOnSave()
{
	FExDataTableImportEditor::RegisterAutoImportOnSave();
}

void FExQuestDataImportEditor::UnregisterAutoImportOnSave()
{
	FExDataTableImportEditor::UnregisterAutoImportOnSave();
}
