// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExK2Node_StreamLevel.h"
#include "BlueprintTool/ExAsyncStreamLevel.h"
#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"

#define LOCTEXT_NAMESPACE "ExK2Node_StreamLevel"

UExK2Node_StreamLevel::UExK2Node_StreamLevel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UExK2Node_StreamLevel::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	
	UEdGraphPin* WorldContextPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UObject::StaticClass(), UEdGraphSchema_K2::PN_Self);
	WorldContextPin->bHidden = true;

	UEdGraphPin* LevelNamePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Name, TEXT("LevelName"));
	LevelNamePin->DefaultValue = TEXT("None");
	
	UEdGraphPin* MakeVisiblePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Boolean, TEXT("bMakeVisibleAfterLoad"));
	MakeVisiblePin->DefaultValue = TEXT("true");
	
	UEdGraphPin* BlockOnSlowPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Boolean, TEXT("bShouldBlockOnSlowLoading"));
	BlockOnSlowPin->DefaultValue = TEXT("false");
}

FText UExK2Node_StreamLevel::GetMenuCategory() const
{
	return LOCTEXT("MenuCategory", "LatentActions|LevelStreaming");
}

FText UExK2Node_StreamLevel::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle_StreamLevel", "Async Stream Level");
}

void UExK2Node_StreamLevel::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* OutputGraph)
{
	Super::ExpandNode(CompilerContext, OutputGraph);
}

#undef LOCTEXT_NAMESPACE
