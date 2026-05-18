// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExK2Node_WaitCount.h"

#include "BlueprintTool/ExWaitBranchProxy.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "ExK2Node_WaitCount"

UExK2Node_WaitCount::UExK2Node_WaitCount(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, CreateProxy_WaitCount);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, SetK2NodeInfo);
	ProxyFactoryClass = UExWaitBranchProxy::StaticClass();
	ProxyClass = UExWaitBranchProxy::StaticClass();
}

FText UExK2Node_WaitCount::GetTooltipText() const
{
	return NSLOCTEXT("ExK2Node_WaitCount", "Tooltip",
		"成功报告（Activate）次数达到 Required Success Count 后沿 Then 继续；失败报告不计入成功次数。对应完成模式 Count。");
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
