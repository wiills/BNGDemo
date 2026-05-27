// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Import/IExDataTableImportHandler.h"

/** Quest task table (FExQuestTaskTableRow) → UExQuestDataAsset import handler. */
class FExQuestDataTableImportHandler : public IExDataTableImportHandler
{
public:
	virtual FName GetHandlerId() const override;
	virtual bool CanImportTable(const UDataTable* Table) const override;
	virtual UClass* GetTargetDataAssetClass() const override;
	virtual bool IsAutoImportOnSaveEnabled() const override;
	virtual FExDataTableImportResult ImportToDataAsset(UDataTable* Table, UObject* TargetAsset) const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetMenuTooltip() const override;
	virtual void InitializeNewTargetAsset(UObject* TargetAsset, const UDataTable* SourceTable) const override;
};
