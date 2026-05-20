// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestDataImportEditor.h"

#include "Async/Async.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "IAssetTools.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackageName.h"
#include "FileHelpers.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Quest/ExQuestDataImport.h"
#include "Quest/ExQuestDefinition.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "Engine/DataTable.h"
#include "UObject/ObjectSaveContext.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "ExQuestDataImportEditor"

namespace ExQuestDataImportEditorInternal
{
	static const TCHAR* QuestImportConfigIni = TEXT("BlueprintNodeGraph");
	static const TCHAR* QuestImportConfigSection = TEXT("ExQuestDataImport");
	static const TCHAR* AutoImportOnSaveKey = TEXT("bAutoImportQuestTableOnSave");

	static FDelegateHandle ObjectSavedDelegateHandle;
	static TSet<TWeakObjectPtr<UDataTable>> PendingAutoImportTables;

	static const FName ContentBrowserQuestImportMenuOwner = TEXT("BlueprintNodeGraphQuestImport");

	static bool CanImportQuestTable(const UDataTable* Table)
	{
		return FExQuestDataImportUtil::IsCompatibleQuestTaskTable(Table);
	}

	static void ExecuteImportForTables(const TArray<UDataTable*>& Tables, bool bPromptOnSave);

	static bool IsAutoImportOnSaveEnabled()
	{
		bool bEnabled = true;
		if (GConfig)
		{
			GConfig->GetBool(
				QuestImportConfigSection,
				AutoImportOnSaveKey,
				bEnabled,
				QuestImportConfigIni);
		}
		return bEnabled;
	}

	static void FlushPendingAutoImports()
	{
		if (PendingAutoImportTables.Num() == 0)
		{
			return;
		}

		TArray<UDataTable*> TablesToImport;
		for (const TWeakObjectPtr<UDataTable>& WeakTable : PendingAutoImportTables)
		{
			if (UDataTable* Table = WeakTable.Get())
			{
				TablesToImport.AddUnique(Table);
			}
		}
		PendingAutoImportTables.Empty();

		if (TablesToImport.Num() == 0)
		{
			return;
		}

		ExecuteImportForTables(TablesToImport, false);
	}

	static void QueueAutoImportAfterSave(UDataTable* TaskTable)
	{
		if (!TaskTable || !IsAutoImportOnSaveEnabled() || !CanImportQuestTable(TaskTable))
		{
			return;
		}

		PendingAutoImportTables.Add(TaskTable);

		AsyncTask(ENamedThreads::GameThread, []()
		{
			FlushPendingAutoImports();
		});
	}

	static void OnPackageSavedWithContext(
		const FString& /*PackageFileName*/,
		UPackage* Package,
		FObjectPostSaveContext SaveContext)
	{
		if (!Package || SaveContext.IsFromAutoSave())
		{
			return;
		}

		ForEachObjectWithPackage(Package, [](UObject* PackageObject)
		{
			if (UDataTable* TaskTable = Cast<UDataTable>(PackageObject))
			{
				QueueAutoImportAfterSave(TaskTable);
			}
			return true;
		}, false);
	}

	static UExQuestDataAsset* FindPairedInRegistry(const FString& DataAssetName, const FString& PreferredPackagePath)
	{
		const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		const FString PreferredObjectPath = PreferredPackagePath + TEXT("/") + DataAssetName + TEXT(".") + DataAssetName;
		if (const FAssetData PreferredAsset = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(PreferredObjectPath));
			PreferredAsset.IsValid())
		{
			return Cast<UExQuestDataAsset>(PreferredAsset.GetAsset());
		}

		TArray<FAssetData> Assets;
		AssetRegistry.GetAssetsByClass(UExQuestDataAsset::StaticClass()->GetClassPathName(), Assets, true);

		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == DataAssetName)
			{
				return Cast<UExQuestDataAsset>(Asset.GetAsset());
			}
		}

		return nullptr;
	}

	/** Persist imported DA only (DT content is unchanged by import). */
	static void SaveImportedDataAssetPackage(UExQuestDataAsset* QuestAsset, bool bPromptOnSave)
	{
		if (!QuestAsset)
		{
			return;
		}

		UPackage* Package = QuestAsset->GetOutermost();
		if (!Package || !Package->IsDirty())
		{
			return;
		}

		TArray<UPackage*> PackagesToSave;
		PackagesToSave.Add(Package);
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, bPromptOnSave);
	}

	static void ExecuteImportForTables(const TArray<UDataTable*>& Tables, bool bPromptOnSave)
	{
		int32 SuccessCount = 0;
		FString CombinedMessage;

		for (UDataTable* Table : Tables)
		{
			if (!Table)
			{
				continue;
			}

			FString Message;
			if (FExQuestDataImportEditor::ImportTableToPairedDataAsset(Table, true, Message, bPromptOnSave))
			{
				++SuccessCount;
			}

			CombinedMessage += Message;
			CombinedMessage += TEXT("\n");
		}

		const FText TitleText = FText::Format(
			LOCTEXT("ImportQuestTableResultTitle", "Quest import: {0} / {1} succeeded"),
			FText::AsNumber(SuccessCount),
			FText::AsNumber(Tables.Num()));

		if (SuccessCount == Tables.Num() && Tables.Num() > 0)
		{
			FNotificationInfo Info(TitleText);
			Info.SubText = FText::FromString(CombinedMessage.TrimEnd());
			Info.ExpireDuration = 5.0f;
			Info.bUseLargeFont = false;
			Info.Image = FAppStyle::GetBrush(TEXT("Icons.Success"));
			FSlateNotificationManager::Get().AddNotification(Info);
		}
		else
		{
			const FText DialogText = FText::Format(
				LOCTEXT("ImportQuestTableResult", "Quest import finished.\nSucceeded: {0} / {1}\n\n{2}"),
				FText::AsNumber(SuccessCount),
				FText::AsNumber(Tables.Num()),
				FText::FromString(CombinedMessage));

			FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		}
	}

	static void ExtendAssetContextMenu(FToolMenuSection& Section, TArray<UDataTable*> SelectedTables)
	{
		TArray<UDataTable*> CompatibleTables;
		for (UDataTable* Table : SelectedTables)
		{
			if (CanImportQuestTable(Table))
			{
				CompatibleTables.Add(Table);
			}
		}

		if (CompatibleTables.Num() == 0)
		{
			return;
		}

		Section.AddMenuEntry(
			"ImportQuestTableToDataAsset",
			LOCTEXT("ImportQuestTableToDataAsset", "Import To Paired Quest Data Asset"),
			LOCTEXT("ImportQuestTableToDataAssetTooltip", "Import rows into DA_* paired with DT_* (e.g. DT_Quest_TestMap → DA_Quest_TestMap). Creates the Data Asset in the same folder if missing."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([CompatibleTables]()
			{
				ExecuteImportForTables(CompatibleTables, true);
			})));
	}

	static void OnExtendContentBrowserAssetSelectionMenu(UToolMenu* Menu)
	{
		if (!Menu)
		{
			return;
		}

		const UContentBrowserAssetContextMenuContext* Context = Menu->FindContext<UContentBrowserAssetContextMenuContext>();
		if (!Context || !Context->bCanBeModified || Context->SelectedAssets.Num() == 0)
		{
			return;
		}

		TArray<UDataTable*> SelectedTables;
		for (const FAssetData& AssetData : Context->SelectedAssets)
		{
			if (UDataTable* Table = Cast<UDataTable>(AssetData.GetAsset()))
			{
				SelectedTables.Add(Table);
			}
		}

		if (SelectedTables.Num() == 0)
		{
			return;
		}

		FToolMenuSection& Section = Menu->AddSection(
			"BlueprintNodeGraphQuestImport",
			LOCTEXT("QuestImportSection", "Quest Import"));

		ExtendAssetContextMenu(Section, SelectedTables);
	}
}

UExQuestDataAsset* FExQuestDataImportEditor::FindPairedDataAsset(const UDataTable* TaskTable)
{
	if (!TaskTable)
	{
		return nullptr;
	}

	const FString DataAssetName = FExQuestDataImportUtil::GetDataAssetNameForTable(TaskTable);
	const FString PackagePath = FPackageName::GetLongPackagePath(TaskTable->GetOutermost()->GetName());
	return ExQuestDataImportEditorInternal::FindPairedInRegistry(DataAssetName, PackagePath);
}

UExQuestDataAsset* FExQuestDataImportEditor::FindOrCreatePairedDataAsset(const UDataTable* TaskTable, bool& bOutCreatedNew)
{
	bOutCreatedNew = false;

	if (!TaskTable)
	{
		return nullptr;
	}

	if (UExQuestDataAsset* Existing = FindPairedDataAsset(TaskTable))
	{
		return Existing;
	}

	const FString DataAssetName = FExQuestDataImportUtil::GetDataAssetNameForTable(TaskTable);
	const FString PackagePath = FPackageName::GetLongPackagePath(TaskTable->GetOutermost()->GetName());

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UExQuestDataAsset* NewAsset = Cast<UExQuestDataAsset>(AssetToolsModule.Get().CreateAsset(
		DataAssetName,
		PackagePath,
		UExQuestDataAsset::StaticClass(),
		nullptr));

	if (NewAsset)
	{
		bOutCreatedNew = true;
		if (NewAsset->QuestSetName.IsEmpty())
		{
			NewAsset->QuestSetName = FText::FromString(DataAssetName);
		}
	}

	return NewAsset;
}

bool FExQuestDataImportEditor::ImportTableToPairedDataAsset(
	UDataTable* TaskTable,
	bool bSavePackages,
	FString& OutMessage,
	bool bPromptOnSave)
{
	OutMessage.Reset();

	if (!TaskTable)
	{
		OutMessage = TEXT("DataTable is null");
		return false;
	}

	if (!ExQuestDataImportEditorInternal::CanImportQuestTable(TaskTable))
	{
		OutMessage = FString::Printf(
			TEXT("'%s' is not a quest task table (row struct must be FExQuestTaskTableRow)."),
			*TaskTable->GetName());
		return false;
	}

	bool bCreatedNew = false;
	UExQuestDataAsset* QuestAsset = FindOrCreatePairedDataAsset(TaskTable, bCreatedNew);
	if (!QuestAsset)
	{
		OutMessage = FString::Printf(TEXT("Failed to find or create paired Data Asset for '%s'."), *TaskTable->GetName());
		return false;
	}

	TArray<FExQuestTaskDefinition> Definitions;
	int32 SkippedRows = 0;
	if (!FExQuestDataImportUtil::GatherTaskDefinitionsFromTable(TaskTable, Definitions, SkippedRows))
	{
		OutMessage = TEXT("Failed to read task rows from DataTable.");
		return false;
	}

	const FExQuestDataImportResult ImportResult =
		FExQuestDataImportUtil::ApplyDefinitionsToDataAsset(QuestAsset, Definitions, TaskTable);

	OutMessage = FString::Printf(
		TEXT("%s (%s)%s Skipped rows: %d"),
		*ImportResult.Message,
		*QuestAsset->GetPathName(),
		bCreatedNew ? TEXT(" [Created]") : TEXT(""),
		SkippedRows);

	if (bSavePackages)
	{
		ExQuestDataImportEditorInternal::SaveImportedDataAssetPackage(QuestAsset, bPromptOnSave);
	}

	return ImportResult.bSuccess;
}

void FExQuestDataImportEditor::RegisterContentBrowserMenus()
{
	if (!UToolMenus::IsToolMenuUIEnabled())
	{
		return;
	}

	UToolMenus* ToolMenus = UToolMenus::Get();
	if (!ToolMenus)
	{
		return;
	}

	UToolMenu* Menu = ToolMenus->ExtendMenu(TEXT("ContentBrowser.AssetContextMenu"));
	if (!Menu)
	{
		return;
	}

	Menu->AddDynamicSection(
		"BlueprintNodeGraphQuestImportDynamic",
		FNewToolMenuDelegate::CreateStatic(&ExQuestDataImportEditorInternal::OnExtendContentBrowserAssetSelectionMenu),
		FToolMenuInsert(FName("GetAssetActions"), EToolMenuInsertType::After));
}

void FExQuestDataImportEditor::UnregisterContentBrowserMenus()
{
	if (UToolMenus* ToolMenus = UToolMenus::Get())
	{
		ToolMenus->UnregisterOwnerByName(ExQuestDataImportEditorInternal::ContentBrowserQuestImportMenuOwner);
	}
}

void FExQuestDataImportEditor::RegisterAutoImportOnSave()
{
	if (ExQuestDataImportEditorInternal::ObjectSavedDelegateHandle.IsValid())
	{
		return;
	}

	ExQuestDataImportEditorInternal::ObjectSavedDelegateHandle =
		UPackage::PackageSavedWithContextEvent.AddStatic(&ExQuestDataImportEditorInternal::OnPackageSavedWithContext);
}

void FExQuestDataImportEditor::UnregisterAutoImportOnSave()
{
	if (ExQuestDataImportEditorInternal::ObjectSavedDelegateHandle.IsValid())
	{
		UPackage::PackageSavedWithContextEvent.Remove(ExQuestDataImportEditorInternal::ObjectSavedDelegateHandle);
		ExQuestDataImportEditorInternal::ObjectSavedDelegateHandle.Reset();
	}

	ExQuestDataImportEditorInternal::PendingAutoImportTables.Empty();
}

#undef LOCTEXT_NAMESPACE
