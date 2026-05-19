// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/K2Nodes/ExK2Node_WaitCondition.h"
#include "BlueprintTool/Proxies/ExProxy_WaitCondition.h"

#define LOCTEXT_NAMESPACE "UK2Node_BaseAsyncTask"


UExK2Node_WaitCondition::UExK2Node_WaitCondition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitCondition, CreateProxy);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitCondition, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitCondition, SetK2NodeInfo);
	ProxyFactoryClass = UExProxy_WaitCondition::StaticClass();
	ProxyClass = UExProxy_WaitCondition::StaticClass();
}

FText UExK2Node_WaitCondition::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "WaitCondition_Tooltip", "Wait Multi-Async-Input-Branches && Condition Satisfired, Then Execute Once");
}

FText UExK2Node_WaitCondition::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "WaitCondition", "Wait Condition");
}

FSlateIcon UExK2Node_WaitCondition::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Sequence_16x");
	return Icon;
}

void UExK2Node_WaitCondition::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
}

#undef LOCTEXT_NAMESPACE
