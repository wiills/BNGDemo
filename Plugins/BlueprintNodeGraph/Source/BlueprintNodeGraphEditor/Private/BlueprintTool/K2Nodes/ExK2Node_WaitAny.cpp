// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/K2Nodes/ExK2Node_WaitAny.h"

#include "BlueprintTool/Proxies/ExProxy_WaitBranch.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "ExK2Node_WaitAny"

UExK2Node_WaitAny::UExK2Node_WaitAny(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitBranch, CreateProxy_WaitAny);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitBranch, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_WaitBranch, SetK2NodeInfo);
	ProxyFactoryClass = UExProxy_WaitBranch::StaticClass();
	ProxyClass = UExProxy_WaitBranch::StaticClass();
}

FText UExK2Node_WaitAny::GetTooltipText() const
{
	return NSLOCTEXT("ExK2Node_WaitAny", "Tooltip",
		"Continues on Then after any branch Activate (success report). Other branches need not complete. Mode: Any.");
}

FText UExK2Node_WaitAny::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	(void)TitleType;
	return NSLOCTEXT("ExK2Node_WaitAny", "Title", "Wait Any");
}

FSlateIcon UExK2Node_WaitAny::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "GraphEditor.Sequence_16x");
	return Icon;
}

FText UExK2Node_WaitAny::GetMenuCategory() const
{
	return NSLOCTEXT("ExK2Node_WaitAny", "MenuCategory", "Blueprint Node Graph|Flow Control");
}

#undef LOCTEXT_NAMESPACE
