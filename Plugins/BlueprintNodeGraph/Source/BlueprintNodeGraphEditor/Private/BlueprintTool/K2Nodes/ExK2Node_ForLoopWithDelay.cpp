// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/K2Nodes/ExK2Node_ForLoopWithDelay.h"
#include "BlueprintTool/Proxies/ExProxy_ForLoopWithDelay.h"

#define LOCTEXT_NAMESPACE "UK2Node_BaseAsyncTask"

namespace ForLoopWithDelayHelper
{
	const FName UUIDPinName = TEXT("UUID");
}

UExK2Node_ForLoopWithDelay::UExK2Node_ForLoopWithDelay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_ForLoopWithDelay, CreateProxy);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_ForLoopWithDelay, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExProxy_ForLoopWithDelay, SetK2NodeInfo);
	ProxyFactoryClass = UExProxy_ForLoopWithDelay::StaticClass();
	ProxyClass = UExProxy_ForLoopWithDelay::StaticClass();
}

FText UExK2Node_ForLoopWithDelay::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "LoopDelay_Tooltip", "For Loop Count, With Loop Interval\n, only execute once when delay running");
}

FText UExK2Node_ForLoopWithDelay::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "ForLoopWithDelay", "For Loop With Delay");
}

FSlateIcon UExK2Node_ForLoopWithDelay::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Macro.Loop_16x");
	return Icon;
}

// void UExK2Node_ForLoopWithDelay::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
// {
// 	// SetValue: UUID && InputCount
// 	{
// 		// uuid
// 		UEdGraphPin* UUIDPin = FindPin(LatentTaskHelper::UUIDPinName);
// 		if (UUIDPin)
// 		{
// 			UUIDPin->DefaultValue = GetNodeUUID();
// 		}
// 		// input count
// 		const int32 ExecCount = GetInputBranchCount();
// 		UEdGraphPin* InputCountPin = FindPin(LatentTaskHelper::InputCountPinName);
// 		if (InputCountPin)
// 		{
// 			InputCountPin->DefaultValue = UKismetStringLibrary::Conv_IntToString(ExecCount);
// 		}
// 	}
// 	
// 	Super::ExpandNode(CompilerContext, SourceGraph);
// }

#undef LOCTEXT_NAMESPACE
