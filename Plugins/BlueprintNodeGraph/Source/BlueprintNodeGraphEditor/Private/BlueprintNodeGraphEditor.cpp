// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintNodeGraphEditor.h"

#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"
#include "BlueprintTool/AssetActions/ExAssetTypeActions_FlowGraph.h"
#include "Quest/ExQuestDataImportEditor.h"
#include "IAssetTools.h"

#define LOCTEXT_NAMESPACE "FBlueprintNodeGraphEditorModule"

namespace
{
	EAssetTypeCategories::Type BlueprintNodeGraphAssetCategory = EAssetTypeCategories::Misc;
	TArray<TSharedPtr<IAssetTypeActions>> RegisteredAssetTypeActions;
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

	FExQuestDataImportEditor::RegisterContentBrowserMenus();
	FExQuestDataImportEditor::RegisterAutoImportOnSave();
}

void FBlueprintNodeGraphEditorModule::ShutdownModule()
{
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

	FExQuestDataImportEditor::UnregisterContentBrowserMenus();
	FExQuestDataImportEditor::UnregisterAutoImportOnSave();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FBlueprintNodeGraphEditorModule, BlueprintNodeGraphEditor)
