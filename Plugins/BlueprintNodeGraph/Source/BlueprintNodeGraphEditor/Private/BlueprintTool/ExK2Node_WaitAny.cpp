// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExK2Node_WaitAny.h"

#include "BlueprintTool/ExWaitBranchProxy.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "ExK2Node_WaitAny"

UExK2Node_WaitAny::UExK2Node_WaitAny(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, CreateProxy_WaitAny);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, SetK2NodeInfo);
	ProxyFactoryClass = UExWaitBranchProxy::StaticClass();
	ProxyClass = UExWaitBranchProxy::StaticClass();
}

FText UExK2Node_WaitAny::GetTooltipText() const
{
	return NSLOCTEXT("ExK2Node_WaitAny", "Tooltip",
		"任意一路 Activate（成功报告）后即沿 Then 继续，无需等待其它分支。对应完成模式 Any。");
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
