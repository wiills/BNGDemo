// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/K2Nodes/ExK2Node_AsyncBlendPercent.h"
#include "BlueprintTool/Proxies/ExProxy_BlendPercent.h"


#define LOCTEXT_NAMESPACE "UExK2Node_AsyncBlendPercent"


UExK2Node_AsyncBlendPercent::UExK2Node_AsyncBlendPercent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_BlendPercent, CreateProxy);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_BlendPercent, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_BlendPercent, SetK2NodeInfo);
	ProxyFactoryClass = UExProxy_BlendPercent::StaticClass();
	ProxyClass = UExProxy_BlendPercent::StaticClass();
}

FText UExK2Node_AsyncBlendPercent::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "AsyncBlendPercent", "AsyncBlendPercent");
}

FText UExK2Node_AsyncBlendPercent::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "AsyncBlendPercent", "AsyncBlendPercent");
}

FSlateIcon UExK2Node_AsyncBlendPercent::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Event_16x");
	return Icon;
}
#undef LOCTEXT_NAMESPACE
