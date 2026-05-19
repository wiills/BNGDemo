// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/K2Nodes/ExK2Node_WaitCount.h"

#include "BlueprintTool/Proxies/ExProxy_WaitBranch.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "ExK2Node_WaitCount"

UExK2Node_WaitCount::UExK2Node_WaitCount(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitBranch, CreateProxy_WaitCount);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitBranch, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitBranch, SetK2NodeInfo);
	ProxyFactoryClass = UExProxy_WaitBranch::StaticClass();
	ProxyClass = UExProxy_WaitBranch::StaticClass();
}

FText UExK2Node_WaitCount::GetTooltipText() const
{
	return NSLOCTEXT("ExK2Node_WaitCount", "Tooltip",
		"Continues on Then after successful Activate reports reach Required Success Count. Failed reports do not count. Mode: Count.");
}

FText UExK2Node_WaitCount::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	(void)TitleType;
	return NSLOCTEXT("ExK2Node_WaitCount", "Title", "Wait Count");
}

FSlateIcon UExK2Node_WaitCount::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "GraphEditor.Sequence_16x");
	return Icon;
}

FText UExK2Node_WaitCount::GetMenuCategory() const
{
	return NSLOCTEXT("ExK2Node_WaitCount", "MenuCategory", "Blueprint Node Graph|Flow Control");
}

#undef LOCTEXT_NAMESPACE
