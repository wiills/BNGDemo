// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintTool/ExK2Node_ShowBase.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintTool/SGraphNode_ShowBase.h"


#define LOCTEXT_NAMESPACE "UK2Node_ShowBase"

FText UK2Node_ShowBase::GetTooltipText() const
{
	return FText::FromString(TEXT("UK2Node_ShowBase ToolTip"));
}

FText UK2Node_ShowBase::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("UK2Node_ShowBase GetNodeTitle"));
}

FText UK2Node_ShowBase::GetMenuCategory() const
{
	return FText::FromString(TEXT("ShowBaseNode"));
}

void UK2Node_ShowBase::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();

	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		// 添加过就可以在蓝图中显示了
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}

	Super::GetMenuActions(ActionRegistrar);
}

void UK2Node_ShowBase::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
}

UEdGraphPin* UK2Node_ShowBase::GetThenPin() const
{
	UEdGraphPin* Pin = FindPin(UEdGraphSchema_K2::PN_Then);
	check(Pin == nullptr || Pin->Direction == EGPD_Output); // If pin exists, it must be output
	return Pin;
}

TSharedPtr<SGraphNode> UK2Node_ShowBase::CreateVisualWidget()
{
	// 创建自定义的GraphNode样式
	return SNew(SGraphNode_ShowBase, this);
}

#undef LOCTEXT_NAMESPACE
