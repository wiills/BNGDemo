// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExK2Node_WaitAll.h"

#include "BlueprintTool/ExWaitBranchProxy.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "ExK2Node_WaitAll"

UExK2Node_WaitAll::UExK2Node_WaitAll(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, CreateProxy_WaitAll);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, SetK2NodeInfo);
	ProxyFactoryClass = UExWaitBranchProxy::StaticClass();
	ProxyClass = UExWaitBranchProxy::StaticClass();
}

FText UExK2Node_WaitAll::GetTooltipText() const
{
	return NSLOCTEXT("ExK2Node_WaitAll", "Tooltip",
		"全部输入分支都报告后沿 Then 继续（Activate 计成功，ReportBranchFailed 计失败但仍算已报告）。对应完成模式 All。");
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
