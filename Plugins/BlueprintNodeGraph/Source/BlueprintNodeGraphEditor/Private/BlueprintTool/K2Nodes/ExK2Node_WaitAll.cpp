// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/K2Nodes/ExK2Node_WaitAll.h"

#include "BlueprintTool/Proxies/ExProxy_WaitBranch.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "ExK2Node_WaitAll"

UExK2Node_WaitAll::UExK2Node_WaitAll(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitBranch, CreateProxy_WaitAll);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitBranch, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitBranch, SetK2NodeInfo);
	ProxyFactoryClass = UExProxy_WaitBranch::StaticClass();
	ProxyClass = UExProxy_WaitBranch::StaticClass();
}

FText UExK2Node_WaitAll::GetTooltipText() const
{
	return NSLOCTEXT("ExK2Node_WaitAll", "Tooltip",
		"Continues on Then after all input branches report. Activate counts success; ReportBranchFailed counts failure but still counts as reported. Mode: All.");
}

FText UExK2Node_WaitAll::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	(void)TitleType;
	return NSLOCTEXT("ExK2Node_WaitAll", "Title", "Wait All");
}

FSlateIcon UExK2Node_WaitAll::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "GraphEditor.Sequence_16x");
	return Icon;
}

FText UExK2Node_WaitAll::GetMenuCategory() const
{
	return NSLOCTEXT("ExK2Node_WaitAll", "MenuCategory", "Blueprint Node Graph|Flow Control");
}

#undef LOCTEXT_NAMESPACE
