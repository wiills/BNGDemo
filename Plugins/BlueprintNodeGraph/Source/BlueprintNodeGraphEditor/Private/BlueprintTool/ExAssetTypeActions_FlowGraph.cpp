// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExAssetTypeActions_FlowGraph.h"
#include "BlueprintTool/ExGraphAsset.h"
#include "Editor.h"
#include "Subsystems/AssetEditorSubsystem.h"

FExAssetTypeActions_FlowGraph::FExAssetTypeActions_FlowGraph(EAssetTypeCategories::Type InCategory)
	: Category(InCategory)
{
}

FText FExAssetTypeActions_FlowGraph::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_ExFlowGraph", "Flow Graph");
}

FColor FExAssetTypeActions_FlowGraph::GetTypeColor() const
{
	return FColor(100, 200, 255);
}

UClass* FExAssetTypeActions_FlowGraph::GetSupportedClass() const
{
	return UExGraphAsset::StaticClass();
}

void FExAssetTypeActions_FlowGraph::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
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

uint32 FExAssetTypeActions_FlowGraph::GetCategories()
{
	return Category;
}

const TArray<FText>& FExAssetTypeActions_FlowGraph::GetSubMenus() const
{
	static const TArray<FText> SubMenus = {
		NSLOCTEXT("AssetTypeActions", "SubMenu_Blueprint", "Blueprint")
	};
	return SubMenus;
}

FText FExAssetTypeActions_FlowGraph::GetAssetDescription(const FAssetData& AssetData) const
{
	return NSLOCTEXT("AssetTypeActions", "Description_ExFlowGraph", "Flow control graph asset for visual behavior trees and task flows");
}

UFactory_ExFlowGraph::UFactory_ExFlowGraph(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UExGraphAsset::StaticClass();
}

UObject* UFactory_ExFlowGraph::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	UExGraphAsset* NewAsset = NewObject<UExGraphAsset>(InParent, Class, Name, Flags | RF_Transactional);
	if (NewAsset)
	{
		NewAsset->GraphName = FText::FromName(Name);
		NewAsset->bAutoStart = false;
		NewAsset->DefaultTimeout = 30.f;
	}
	return NewAsset;
}

FText UFactory_ExFlowGraph::GetDisplayName() const
{
	return NSLOCTEXT("Factory", "Factory_ExFlowGraph", "Flow Graph Asset");
}

uint32 UFactory_ExFlowGraph::GetMenuCategories() const
{
	return EAssetTypeCategories::Blueprint;
}

FName UFactory_ExFlowGraph::GetNewAssetThumbnailOverride() const
{
	return FName(TEXT("BlueprintThumbnail"));
}

bool UFactory_ExFlowGraph::ShouldShowInNewMenu() const
{
	return true;
}
