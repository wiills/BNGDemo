// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintNodeGraphEditor.h"

#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"
#include "BlueprintTool/AssetActions/ExAssetTypeActions_FlowGraph.h"
#include "BlueprintTool/Details/ExClassedAsyncNodeDetails.h"
#include "BlueprintTool/K2Nodes/ExK2Node_ClassedAsyncBase.h"
#include "Import/ExDataTableImportEditor.h"
#include "IAssetTools.h"
#include "PropertyEditorModule.h"
#include "Quest/ExQuestDataTableImportHandler.h"

#define LOCTEXT_NAMESPACE "FBlueprintNodeGraphEditorModule"

namespace
{
	EAssetTypeCategories::Type BlueprintNodeGraphAssetCategory = EAssetTypeCategories::Misc;
	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;
	TSharedPtr<FExQuestDataTableImportHandler> QuestDataTableImportHandler;
}

void FBlueprintNodeGraphEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	BlueprintNodeGraphAssetCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("BlueprintNodeGraph")),
		LOCTEXT("BlueprintNodeGraphAssetCategory", "Blueprint Node Graph"));

	RegisteredAssetTypeActions.Add(MakeShareable(new FExAssetTypeActions_FlowGraph(BlueprintNodeGraphAssetCategory)));

	for (const TSharedPtr<IAssetTypeActions>& Action : RegisteredAssetTypeActions)
	{
		if (Action.IsValid())
		{
			AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());
		}
	}

	QuestDataTableImportHandler = MakeShared<FExQuestDataTableImportHandler>();
	FExDataTableImportEditor::RegisterHandler(QuestDataTableImportHandler.ToSharedRef());
	FExDataTableImportEditor::RegisterContentBrowserMenus();
	FExDataTableImportEditor::RegisterAutoImportOnSave();

	// Register Detail Customization for classed async nodes
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomClassLayout(
		UExK2Node_ClassedAsyncBase::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FExClassedAsyncNodeDetails::MakeInstance)
	);
}

void FBlueprintNodeGraphEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomClassLayout(UExK2Node_ClassedAsyncBase::StaticClass()->GetFName());
	}

	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (const TSharedPtr<IAssetTypeActions>& Action : RegisteredAssetTypeActions)
		{
			if (Action.IsValid())
			{
				AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
			}
		}
	}
	RegisteredAssetTypeActions.Empty();

	FExDataTableImportEditor::UnregisterAutoImportOnSave();
	FExDataTableImportEditor::UnregisterContentBrowserMenus();
	FExDataTableImportEditor::UnregisterAllHandlers();
	QuestDataTableImportHandler.Reset();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBlueprintNodeGraphEditorModule, BlueprintNodeGraphEditor)
