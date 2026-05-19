// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintTool/K2Nodes/ExK2Node_SwitchValue.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintTool/Slate/SGraphNode_ShowBase.h"


#define LOCTEXT_NAMESPACE "UExK2Node_SwitchValue"


UExK2Node_SwitchValue::UExK2Node_SwitchValue(const FObjectInitializer& ObjectInitializer)
{
}

FText UExK2Node_SwitchValue::GetTooltipText() const
{
	return FText::FromString(TEXT("SwitchValue ToolTip"));
}

FText UExK2Node_SwitchValue::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("SwitchValue GetNodeTitle"));
}

FText UExK2Node_SwitchValue::GetMenuCategory() const
{
	return FText::FromString(TEXT("SwitchValue"));
}

void UExK2Node_SwitchValue::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();

	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		// ?
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}

	Super::GetMenuActions(ActionRegistrar);
}

void UExK2Node_SwitchValue::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	// exec pin
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	// default input value pin
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Int, TEXT("InputValue"));
}

void UExK2Node_SwitchValue::AddPinToNode()
{
	Super::AddPinToNode();

	const auto InName = *FString::Printf(TEXT("%s_%d"), TEXT("Param"), Index);
	const auto OutName = *FString::Printf(TEXT("%s_%d"), TEXT("OnParam"), Index);
	Index++;
	
	Modify();
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Int, InName);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, OutName);
}

TSharedPtr<SGraphNode> UExK2Node_SwitchValue::CreateVisualWidget()
{
	// GraphNode
	return SNew(SGraphNode_AddInputPin, this);
}

FName UExK2Node_SwitchValue::GetPinNameGivenIndex(int32 InIndex) const
{
	return *FString::Printf(TEXT("%s_%d"), *UEdGraphSchema_K2::PN_Then.ToString(), InIndex);
}

void UExK2Node_SwitchValue::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);
}

#undef LOCTEXT_NAMESPACE
