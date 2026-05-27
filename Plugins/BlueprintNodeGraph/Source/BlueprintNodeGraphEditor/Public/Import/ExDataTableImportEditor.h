// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UDataTable;
class IExDataTableImportHandler;
class UObject;

/** Editor-only generic shell: handler registry, paired DA lookup, auto-import on save, Content Browser menus. */
class FExDataTableImportEditor
{
public:
	static void RegisterHandler(TSharedRef<IExDataTableImportHandler> Handler);
	static void UnregisterAllHandlers();

	static const IExDataTableImportHandler* ResolveHandler(const UDataTable* Table);

	static UObject* FindPairedDataAsset(const UDataTable* Table, const IExDataTableImportHandler& Handler);

	static UObject* FindOrCreatePairedDataAsset(
		const UDataTable* Table,
		const IExDataTableImportHandler& Handler,
		bool& bOutCreatedNew);

	static bool ImportTableToPairedDataAsset(
		UDataTable* Table,
		bool bSavePackages,
		FString& OutMessage,
		bool bPromptOnSave = true);

	static void RegisterContentBrowserMenus();
	static void UnregisterContentBrowserMenus();

	static void RegisterAutoImportOnSave();
	static void UnregisterAutoImportOnSave();
};
