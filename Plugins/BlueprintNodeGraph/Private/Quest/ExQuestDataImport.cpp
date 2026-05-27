// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestDataImport.h"

#include "Quest/ExQuestDefinition.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"

namespace ExQuestDataImport
{
	static const FString TablePrefix = TEXT("DT_");
	static const FString AssetPrefix = TEXT("DA_");
}

FString FExQuestDataImportUtil::GetDataAssetNameForTableAssetName(const FString& TableAssetName)
{
	if (TableAssetName.StartsWith(ExQuestDataImport::TablePrefix))
	{
		return ExQuestDataImport::AssetPrefix + TableAssetName.Mid(ExQuestDataImport::TablePrefix.Len());
	}

	return ExQuestDataImport::AssetPrefix + TableAssetName;
}

FString FExQuestDataImportUtil::GetDataAssetNameForTable(const UDataTable* TaskTable)
{
	if (!TaskTable)
	{
		return FString();
	}

	return GetDataAssetNameForTableAssetName(TaskTable->GetName());
}

bool FExQuestDataImportUtil::IsCompatibleQuestTaskTable(const UDataTable* TaskTable)
{
	return TaskTable && TaskTable->GetRowStruct() == FExQuestTaskTableRow::StaticStruct();
}

bool FExQuestDataImportUtil::GatherTaskDefinitionsFromTable(
	const UDataTable* TaskTable,
	TArray<FExQuestTaskDefinition>& OutDefinitions,
	int32& OutSkippedRows)
{
	OutDefinitions.Reset();
	OutSkippedRows = 0;

	if (!IsCompatibleQuestTaskTable(TaskTable))
	{
		return false;
	}

	TaskTable->ForeachRow<FExQuestTaskTableRow>(TEXT("GatherTaskDefinitionsFromTable"),
		[&OutDefinitions, &OutSkippedRows](const FName& RowName, const FExQuestTaskTableRow& Row)
		{
			if (!Row.TaskId.IsValid())
			{
				UE_LOG(LogBlueprintNodeGraph, Warning,
					TEXT("Quest import: row '%s' has invalid TaskId, skipped"),
					*RowName.ToString());
				++OutSkippedRows;
				return;
			}

			OutDefinitions.Add(Row.ToTaskDefinition());
		});

	return true;
}

FExQuestDataImportResult FExQuestDataImportUtil::ApplyDefinitionsToDataAsset(
	UExQuestDataAsset* QuestAsset,
	const TArray<FExQuestTaskDefinition>& Definitions,
	const UDataTable* SourceTable)
{
	FExQuestDataImportResult Result;

	if (!QuestAsset)
	{
		Result.Message = TEXT("Quest Data Asset is null");
		return Result;
	}

	QuestAsset->Modify();
	QuestAsset->TaskDefinitions = Definitions;
	QuestAsset->QuestSetId = QuestAsset->GetName();

	if (QuestAsset->QuestSetName.IsEmpty())
	{
		QuestAsset->QuestSetName = FText::FromString(QuestAsset->QuestSetId);
	}

#if WITH_EDITOR
	QuestAsset->SetSourceTaskTable(SourceTable);
#endif

	Result.ImportedTaskCount = Definitions.Num();
	Result.bSuccess = true;
	Result.Message = FString::Printf(
		TEXT("Imported %d task(s) into '%s'"),
		Result.ImportedTaskCount,
		*QuestAsset->GetName());

	return Result;
}
