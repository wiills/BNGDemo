// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/ExK2Node_LoopDelay.h"

#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"
#include "BlueprintTool/ExLoopDelayProxy.h"
#include "Misc/StringFormatArg.h"


#define LOCTEXT_NAMESPACE "UK2Node_BaseAsyncTask"


UExK2Node_LoopDelay::UExK2Node_LoopDelay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExLoopDelayProxy, CreateProxy);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExLoopDelayProxy, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExLoopDelayProxy, SetK2NodeInfo);
	ProxyFactoryClass = UExLoopDelayProxy::StaticClass();
	ProxyClass = UExLoopDelayProxy::StaticClass();
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
