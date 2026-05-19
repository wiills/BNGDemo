// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/K2Nodes/ExK2Node_LoopDelay.h"
#include "BlueprintTool/Proxies/ExProxy_LoopDelay.h"

#define LOCTEXT_NAMESPACE "UK2Node_BaseAsyncTask"


UExK2Node_LoopDelay::UExK2Node_LoopDelay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_LoopDelay, CreateProxy);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_LoopDelay, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_LoopDelay, SetK2NodeInfo);
	ProxyFactoryClass = UExProxy_LoopDelay::StaticClass();
	ProxyClass = UExProxy_LoopDelay::StaticClass();
}

FText UExK2Node_LoopDelay::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "LoopDelay_Tooltip", "Wait Then Execute\n, if you use this, you will have at least 1 frame delay");
}

FText UExK2Node_LoopDelay::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "LoopDelay", "DelayInLoop");
}

FSlateIcon UExK2Node_LoopDelay::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Event_16x");
	return Icon;
}

#undef LOCTEXT_NAMESPACE
