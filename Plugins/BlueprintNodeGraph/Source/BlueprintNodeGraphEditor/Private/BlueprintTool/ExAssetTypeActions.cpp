// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExAssetTypeActions.h"
#include "BlueprintTool/ExGraphAsset.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"

const FName ExGraphAssetName = TEXT("ExFlowGraph");

FText FExAssetTypeActions_Graph::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ExGraph", "Flow Graph");
}

FColor FExAssetTypeActions_Graph::GetTypeColor() const
{
	return FColor(100, 200, 255);
}

UClass* FExAssetTypeActions_Graph::GetSupportedClass() const
{
	return UExGraphAsset::StaticClass();
}

void FExAssetTypeActions_Graph::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	if (!GEditor)
	{
		return;
	}
	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		for (UObject* Object : InObjects)
		{
			if (UExGraphAsset* GraphAsset = Cast<UExGraphAsset>(Object))
			{
				AssetEditorSubsystem->OpenEditorForAsset(GraphAsset, EToolkitMode::Standalone, EditWithinLevelEditor);
			}
		}
	}
}

uint32 FExAssetTypeActions_Graph::GetCategories()
{
	return EAssetTypeCategories::Blueprint;
}

FText FExAssetTypeActions_Graph::GetAssetDescription(const FAssetData& AssetData) const
{
	return NSLOCTEXT("AssetTypeActions", "AssetDescription_ExGraph", "A flowchart-based flow control graph asset for visual behavior trees");
}
