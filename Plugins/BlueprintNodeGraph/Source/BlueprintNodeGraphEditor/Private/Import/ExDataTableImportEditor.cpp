// Copyright Epic Games, Inc. All Rights Reserved.

#include "Import/ExDataTableImportEditor.h"

#include "Async/Async.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserMenuContexts.h"
#include "Engine/DataTable.h"
#include "FileHelpers.h"
#include "Framework/Notifications/NotificationManager.h"
#include "IAssetTools.h"
#include "Import/IExDataTableImportHandler.h"
#include "Misc/MessageDialog.h"
#include "Misc/PackageName.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "UObject/ObjectSaveContext.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "ExDataTableImportEditor"

namespace ExDataTableImportEditorInternal
{
	static TArray<TSharedRef<IExDataTableImportHandler>> RegisteredHandlers;
	static FDelegateHandle ObjectSavedDelegateHandle;
	static TSet<TWeakObjectPtr<UDataTable>> PendingAutoImportTables;

	static const FName ContentBrowserImportMenuOwner = TEXT("BlueprintNodeGraphDataTableImport");

	static void ExecuteImportForTables(const TArray<UDataTable*>& Tables, bool bPromptOnSave);
	static void SaveImportedDataAssetPackage(UObject* TargetAsset, bool bPromptOnSave);

	static UObject* FindPairedInRegistry(
		const FString& DataAssetName,
		const FString& PreferredPackagePath,
		UClass* AssetClass)
	{
		if (!AssetClass)
		{
			return nullptr;
		}

		const FAssetRegistryModule& AssetRegistryModule =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		const FString PreferredObjectPath =
			PreferredPackagePath + TEXT("/") + DataAssetName + TEXT(".") + DataAssetName;
		if (const FAssetData PreferredAsset = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(PreferredObjectPath));
			PreferredAsset.IsValid())
		{
			return PreferredAsset.GetAsset();
		}

		TArray<FAssetData> Assets;
		AssetRegistry.GetAssetsByClass(AssetClass->GetClassPathName(), Assets, true);

		for (const FAssetData& Asset : Assets)
		{
			if (Asset.AssetName.ToString() == DataAssetName)
			{
				return Asset.GetAsset();
			}
		}

		return nullptr;
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

	static void QueueAutoImportAfterSave(UDataTable* Table)
	{
		const IExDataTableImportHandler* Handler = FExDataTableImportEditor::ResolveHandler(Table);
		if (!Table || !Handler || !Handler->IsAutoImportOnSaveEnabled())
		{
			return;
		}

		PendingAutoImportTables.Add(Table);

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
			if (UDataTable* Table = Cast<UDataTable>(PackageObject))
			{
				QueueAutoImportAfterSave(Table);
			}
			return true;
		}, false);
	}

	static void SaveImportedDataAssetPackage(UObject* TargetAsset, bool bPromptOnSave)
	{
		if (!TargetAsset)
		{
			return;
		}

		UPackage* Package = TargetAsset->GetOutermost();
		if (!Package || !Package->IsDirty())
		{
			return;
		}

		TArray<UPackage*> PackagesToSave;
		PackagesToSave.Add(Package);
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, bPromptOnSave);
	}

	static void ShowImportResult(int32 SuccessCount, int32 TotalCount, const FString& CombinedMessage)
	{
		const FText TitleText = FText::Format(
			LOCTEXT("ImportTableResultTitle", "DataTable import: {0} / {1} succeeded"),
			FText::AsNumber(SuccessCount),
			FText::AsNumber(TotalCount));

		if (SuccessCount == TotalCount && TotalCount > 0)
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
				LOCTEXT("ImportTableResult", "DataTable import finished.\nSucceeded: {0} / {1}\n\n{2}"),
				FText::AsNumber(SuccessCount),
				FText::AsNumber(TotalCount),
				FText::FromString(CombinedMessage));

			FMessageDialog::Open(EAppMsgType::Ok, DialogText);
		}
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
			if (FExDataTableImportEditor::ImportTableToPairedDataAsset(Table, true, Message, bPromptOnSave))
			{
				++SuccessCount;
			}

			if (!Message.IsEmpty())
			{
				CombinedMessage += Message;
				CombinedMessage += TEXT("\n");
			}
		}

		ShowImportResult(SuccessCount, Tables.Num(), CombinedMessage);
	}

	static void ExtendAssetContextMenu(FToolMenuSection& Section, const TArray<UDataTable*>& SelectedTables)
	{
		for (const TSharedRef<IExDataTableImportHandler>& Handler : RegisteredHandlers)
		{
			TArray<UDataTable*> CompatibleTables;
			for (UDataTable* Table : SelectedTables)
			{
				if (Table && Handler->CanImportTable(Table))
				{
					CompatibleTables.Add(Table);
				}
			}

			if (CompatibleTables.Num() == 0)
			{
				continue;
			}

			const FName MenuEntryName = FName(*FString::Printf(TEXT("ImportDataTable_%s"), *Handler->GetHandlerId().ToString()));
			Section.AddMenuEntry(
				MenuEntryName,
				Handler->GetDisplayName(),
				Handler->GetMenuTooltip(),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([CompatibleTables]()
				{
					ExecuteImportForTables(CompatibleTables, true);
				})));
		}
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

		if (SelectedTables.Num() == 0 || RegisteredHandlers.Num() == 0)
		{
			return;
		}

		FToolMenuSection& Section = Menu->AddSection(
			"BlueprintNodeGraphDataTableImport",
			LOCTEXT("DataTableImportSection", "DataTable Import"));

		ExtendAssetContextMenu(Section, SelectedTables);
	}
}

void FExDataTableImportEditor::RegisterHandler(TSharedRef<IExDataTableImportHandler> Handler)
{
	using namespace ExDataTableImportEditorInternal;

	for (const TSharedRef<IExDataTableImportHandler>& Existing : RegisteredHandlers)
	{
		if (Existing->GetHandlerId() == Handler->GetHandlerId())
		{
			UE_LOG(LogTemp, Warning,
				TEXT("FExDataTableImportEditor: handler '%s' already registered, skipping duplicate."),
				*Handler->GetHandlerId().ToString());
			return;
		}
	}

	RegisteredHandlers.Add(Handler);
	RegisteredHandlers.Sort([](const TSharedRef<IExDataTableImportHandler>& A, const TSharedRef<IExDataTableImportHandler>& B)
	{
		return A->GetPriority() > B->GetPriority();
	});
}

void FExDataTableImportEditor::UnregisterAllHandlers()
{
	ExDataTableImportEditorInternal::RegisteredHandlers.Empty();
}

const IExDataTableImportHandler* FExDataTableImportEditor::ResolveHandler(const UDataTable* Table)
{
	if (!Table)
	{
		return nullptr;
	}

	for (const TSharedRef<IExDataTableImportHandler>& Handler : ExDataTableImportEditorInternal::RegisteredHandlers)
	{
		if (Handler->CanImportTable(Table))
		{
			return &Handler.Get();
		}
	}

	return nullptr;
}

UObject* FExDataTableImportEditor::FindPairedDataAsset(const UDataTable* Table, const IExDataTableImportHandler& Handler)
{
	if (!Table)
	{
		return nullptr;
	}

	const FString DataAssetName = Handler.GetPairedAssetName(Table);
	const FString PackagePath = FPackageName::GetLongPackagePath(Table->GetOutermost()->GetName());
	return ExDataTableImportEditorInternal::FindPairedInRegistry(
		DataAssetName,
		PackagePath,
		Handler.GetTargetDataAssetClass());
}

UObject* FExDataTableImportEditor::FindOrCreatePairedDataAsset(
	const UDataTable* Table,
	const IExDataTableImportHandler& Handler,
	bool& bOutCreatedNew)
{
	bOutCreatedNew = false;

	if (!Table)
	{
		return nullptr;
	}

	if (UObject* Existing = FindPairedDataAsset(Table, Handler))
	{
		return Existing;
	}

	UClass* AssetClass = Handler.GetTargetDataAssetClass();
	if (!AssetClass)
	{
		return nullptr;
	}

	const FString DataAssetName = Handler.GetPairedAssetName(Table);
	const FString PackagePath = FPackageName::GetLongPackagePath(Table->GetOutermost()->GetName());

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
		DataAssetName,
		PackagePath,
		AssetClass,
		nullptr);

	if (NewAsset)
	{
		bOutCreatedNew = true;
		Handler.InitializeNewTargetAsset(NewAsset, Table);
	}

	return NewAsset;
}

bool FExDataTableImportEditor::ImportTableToPairedDataAsset(
	UDataTable* Table,
	bool bSavePackages,
	FString& OutMessage,
	bool bPromptOnSave)
{
	OutMessage.Reset();

	if (!Table)
	{
		OutMessage = TEXT("DataTable is null");
		return false;
	}

	const IExDataTableImportHandler* Handler = ResolveHandler(Table);
	if (!Handler)
	{
		OutMessage = FString::Printf(
			TEXT("'%s' has no registered DataTable import handler."),
			*Table->GetName());
		return false;
	}

	bool bCreatedNew = false;
	UObject* TargetAsset = FindOrCreatePairedDataAsset(Table, *Handler, bCreatedNew);
	if (!TargetAsset)
	{
		OutMessage = FString::Printf(
			TEXT("Failed to find or create paired Data Asset for '%s' (handler '%s')."),
			*Table->GetName(),
			*Handler->GetHandlerId().ToString());
		return false;
	}

	const FExDataTableImportResult ImportResult = Handler->ImportToDataAsset(Table, TargetAsset);

	OutMessage = FString::Printf(
		TEXT("[%s] %s (%s)%s Skipped rows: %d"),
		*Handler->GetHandlerId().ToString(),
		*ImportResult.Message,
		*TargetAsset->GetPathName(),
		bCreatedNew ? TEXT(" [Created]") : TEXT(""),
		ImportResult.SkippedCount);

	if (bSavePackages && ImportResult.bSuccess)
	{
		ExDataTableImportEditorInternal::SaveImportedDataAssetPackage(TargetAsset, bPromptOnSave);
	}

	return ImportResult.bSuccess;
}

void FExDataTableImportEditor::RegisterContentBrowserMenus()
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
		"BlueprintNodeGraphDataTableImportDynamic",
		FNewToolMenuDelegate::CreateStatic(&ExDataTableImportEditorInternal::OnExtendContentBrowserAssetSelectionMenu),
		FToolMenuInsert(FName("GetAssetActions"), EToolMenuInsertType::After));
}

void FExDataTableImportEditor::UnregisterContentBrowserMenus()
{
	if (UToolMenus* ToolMenus = UToolMenus::Get())
	{
		ToolMenus->UnregisterOwnerByName(ExDataTableImportEditorInternal::ContentBrowserImportMenuOwner);
	}
}

void FExDataTableImportEditor::RegisterAutoImportOnSave()
{
	if (ExDataTableImportEditorInternal::ObjectSavedDelegateHandle.IsValid())
	{
		return;
	}

	ExDataTableImportEditorInternal::ObjectSavedDelegateHandle =
		UPackage::PackageSavedWithContextEvent.AddStatic(&ExDataTableImportEditorInternal::OnPackageSavedWithContext);
}

void FExDataTableImportEditor::UnregisterAutoImportOnSave()
{
	if (ExDataTableImportEditorInternal::ObjectSavedDelegateHandle.IsValid())
	{
		UPackage::PackageSavedWithContextEvent.Remove(ExDataTableImportEditorInternal::ObjectSavedDelegateHandle);
		ExDataTableImportEditorInternal::ObjectSavedDelegateHandle.Reset();
	}

	ExDataTableImportEditorInternal::PendingAutoImportTables.Empty();
}

#undef LOCTEXT_NAMESPACE
