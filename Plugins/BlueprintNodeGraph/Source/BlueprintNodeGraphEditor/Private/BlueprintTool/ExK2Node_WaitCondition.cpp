// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/ExK2Node_WaitCondition.h"

#include "K2Node_CallFunction.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"
#include "BlueprintTool/ExWaitConditionProxy.h"
#include "Kismet/KismetStringLibrary.h"


#define LOCTEXT_NAMESPACE "UK2Node_BaseAsyncTask"


UExK2Node_WaitCondition::UExK2Node_WaitCondition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitConditionProxy, CreateProxy);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitConditionProxy, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExWaitConditionProxy, SetK2NodeInfo);
	ProxyFactoryClass = UExWaitConditionProxy::StaticClass();
	ProxyClass = UExWaitConditionProxy::StaticClass();
}

FText UExK2Node_WaitCondition::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "WaitCondition_Tooltip", "Wait Multi-Async-Input-Branches && Condition Satisfired, Then Execute Once");
}

FText UExK2Node_WaitCondition::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "WaitCondition", "Wait Condition");
}

FSlateIcon UExK2Node_WaitCondition::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Sequence_16x");
	return Icon;
}

void UExK2Node_WaitCondition::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
}

#undef LOCTEXT_NAMESPACE
