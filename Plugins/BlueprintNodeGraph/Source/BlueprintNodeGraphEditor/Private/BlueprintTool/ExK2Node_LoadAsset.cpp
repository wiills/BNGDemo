// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExK2Node_LoadAsset.h"
#include "BlueprintTool/ExAsyncLoadAsset.h"
#include "KismetCompiler.h"

#define LOCTEXT_NAMESPACE "ExK2Node_LoadAsset"

UExK2Node_LoadAsset::UExK2Node_LoadAsset(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UExK2Node_LoadAsset::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	
	UEdGraphPin* WorldContextPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), UEdGraphSchema_K2::PN_Self);
	WorldContextPin->bHidden = true;

	UEdGraphPin* AssetPathPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_SoftObject, TEXT("AssetPath"));
	AssetPathPin->DefaultValue = TEXT("None");
	
	UEdGraphPin* AssetClassPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Class, TEXT("AssetClass"));
	AssetClassPin->DefaultValue = TEXT("Object");
}

FText UExK2Node_LoadAsset::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "LatentActions|AssetLoading");
}

FText UExK2Node_LoadAsset::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle_LoadAsset", "Async Load Asset");
}

void UExK2Node_LoadAsset::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* OutputGraph)
{
	Super::ExpandNode(CompilerContext, OutputGraph);
}

#undef LOCTEXT_NAMESPACE
