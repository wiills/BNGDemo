// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UDataTable;
class UObject;

/** Result of importing rows from a DataTable into a paired DataAsset. */
struct FExDataTableImportResult
{
	bool bSuccess = false;
	FString Message;
	int32 ImportedCount = 0;
	int32 SkippedCount = 0;
};

/** Per-feature strategy for DT_* → DA_* editor import. Register with FExDataTableImportEditor. */
class IExDataTableImportHandler
{
public:
	virtual ~IExDataTableImportHandler() = default;

	virtual FName GetHandlerId() const = 0;

	virtual bool CanImportTable(const UDataTable* Table) const = 0;

	virtual UClass* GetTargetDataAssetClass() const = 0;

	/** Default: DT_Foo → DA_Foo (strip DT_ prefix when present). */
	virtual FString GetPairedAssetName(const UDataTable* Table) const;

	virtual bool IsAutoImportOnSaveEnabled() const;

	/** Write converted rows into TargetAsset. Does not save packages. */
	virtual FExDataTableImportResult ImportToDataAsset(UDataTable* Table, UObject* TargetAsset) const = 0;

	virtual FText GetDisplayName() const = 0;

	virtual FText GetMenuTooltip() const;

	/** Called after a new paired DataAsset is created. */
	virtual void InitializeNewTargetAsset(UObject* TargetAsset, const UDataTable* SourceTable) const;

	/** Higher value wins when multiple handlers match the same table. */
	virtual int32 GetPriority() const;
};
