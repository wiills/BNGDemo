// Copyright Epic Games, Inc. All Rights Reserved.

#include "Import/IExDataTableImportHandler.h"

#include "Engine/DataTable.h"

namespace ExDataTableImport
{
	static const FString TablePrefix = TEXT("DT_");
	static const FString AssetPrefix = TEXT("DA_");
}

FString IExDataTableImportHandler::GetPairedAssetName(const UDataTable* Table) const
{
	if (!Table)
	{
		return FString();
	}

	const FString TableAssetName = Table->GetName();
	if (TableAssetName.StartsWith(ExDataTableImport::TablePrefix))
	{
		return ExDataTableImport::AssetPrefix + TableAssetName.Mid(ExDataTableImport::TablePrefix.Len());
	}

	return ExDataTableImport::AssetPrefix + TableAssetName;
}

bool IExDataTableImportHandler::IsAutoImportOnSaveEnabled() const
{
	return true;
}

FText IExDataTableImportHandler::GetMenuTooltip() const
{
	return NSLOCTEXT(
		"ExDataTableImport",
		"DefaultImportMenuTooltip",
		"Import rows into a paired Data Asset in the same folder (DT_* → DA_*). Creates the asset if missing.");
}

void IExDataTableImportHandler::InitializeNewTargetAsset(UObject* /*TargetAsset*/, const UDataTable* /*SourceTable*/) const
{
}

int32 IExDataTableImportHandler::GetPriority() const
{
	return 0;
}
