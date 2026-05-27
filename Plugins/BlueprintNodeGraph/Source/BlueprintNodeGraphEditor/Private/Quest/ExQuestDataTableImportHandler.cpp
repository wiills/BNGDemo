// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestDataTableImportHandler.h"

#include "Import/IExDataTableImportHandler.h"
#include "Misc/ConfigCacheIni.h"
#include "Quest/ExQuestDataImport.h"
#include "Quest/ExQuestDefinition.h"

namespace ExQuestDataTableImportHandlerInternal
{
	static const TCHAR* QuestImportConfigIni = TEXT("BlueprintNodeGraph");
	static const TCHAR* QuestImportConfigSection = TEXT("ExQuestDataImport");
	static const TCHAR* AutoImportOnSaveKey = TEXT("bAutoImportQuestTableOnSave");
}

FName FExQuestDataTableImportHandler::GetHandlerId() const
{
	return TEXT("Quest");
}

bool FExQuestDataTableImportHandler::CanImportTable(const UDataTable* Table) const
{
	return FExQuestDataImportUtil::IsCompatibleQuestTaskTable(Table);
}

UClass* FExQuestDataTableImportHandler::GetTargetDataAssetClass() const
{
	return UExQuestDataAsset::StaticClass();
}

bool FExQuestDataTableImportHandler::IsAutoImportOnSaveEnabled() const
{
	bool bEnabled = true;
	if (GConfig)
	{
		GConfig->GetBool(
			ExQuestDataTableImportHandlerInternal::QuestImportConfigSection,
			ExQuestDataTableImportHandlerInternal::AutoImportOnSaveKey,
			bEnabled,
			ExQuestDataTableImportHandlerInternal::QuestImportConfigIni);
	}
	return bEnabled;
}

FExDataTableImportResult FExQuestDataTableImportHandler::ImportToDataAsset(UDataTable* Table, UObject* TargetAsset) const
{
	FExDataTableImportResult Result;

	UExQuestDataAsset* QuestAsset = Cast<UExQuestDataAsset>(TargetAsset);
	if (!Table || !QuestAsset)
	{
		Result.Message = TEXT("Invalid quest table or data asset");
		return Result;
	}

	TArray<FExQuestTaskDefinition> Definitions;
	int32 SkippedRows = 0;
	if (!FExQuestDataImportUtil::GatherTaskDefinitionsFromTable(Table, Definitions, SkippedRows))
	{
		Result.Message = TEXT("Failed to read task rows from DataTable.");
		return Result;
	}

	const FExQuestDataImportResult QuestResult =
		FExQuestDataImportUtil::ApplyDefinitionsToDataAsset(QuestAsset, Definitions, Table);

	Result.bSuccess = QuestResult.bSuccess;
	Result.Message = QuestResult.Message;
	Result.ImportedCount = QuestResult.ImportedTaskCount;
	Result.SkippedCount = SkippedRows;
	return Result;
}

FText FExQuestDataTableImportHandler::GetDisplayName() const
{
	return NSLOCTEXT("ExQuestDataTableImport", "ImportMenuLabel", "Import To Paired Quest Data Asset");
}

FText FExQuestDataTableImportHandler::GetMenuTooltip() const
{
	return NSLOCTEXT(
		"ExQuestDataTableImport",
		"ImportMenuTooltip",
		"Import rows into DA_* paired with DT_* (e.g. DT_Quest_TestMap → DA_Quest_TestMap). Creates the Data Asset in the same folder if missing.");
}

void FExQuestDataTableImportHandler::InitializeNewTargetAsset(UObject* TargetAsset, const UDataTable* SourceTable) const
{
	UExQuestDataAsset* QuestAsset = Cast<UExQuestDataAsset>(TargetAsset);
	if (!QuestAsset || !SourceTable)
	{
		return;
	}

	if (QuestAsset->QuestSetName.IsEmpty())
	{
		QuestAsset->QuestSetName = FText::FromString(FExQuestDataImportUtil::GetDataAssetNameForTable(SourceTable));
	}
}
