// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/ExK2Node_WaitBranch.h"

#include "K2Node_CallFunction.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"
#include "BlueprintTool/ExWaitBranchProxy.h"
#include "Kismet/KismetStringLibrary.h"


#define LOCTEXT_NAMESPACE "UK2Node_BaseAsyncTask"


UExK2Node_WaitBranch::UExK2Node_WaitBranch(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, CreateProxy);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitBranchProxy, SetK2NodeInfo);
	ProxyFactoryClass = UExWaitBranchProxy::StaticClass();
	ProxyClass = UExWaitBranchProxy::StaticClass();
}

FText UExK2Node_WaitBranch::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "WaitInputAll_Tooltip", "Wait Multi-Async-Input-Branches, Then Execute Once");
}

FText UExK2Node_WaitBranch::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "WaitBranchIn", "Wait Branches");
}

FSlateIcon UExK2Node_WaitBranch::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Sequence_16x");
	return Icon;
}
#undef LOCTEXT_NAMESPACE
