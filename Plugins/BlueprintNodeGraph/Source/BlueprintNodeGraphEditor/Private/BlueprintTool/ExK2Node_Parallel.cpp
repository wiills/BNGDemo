// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExK2Node_Parallel.h"

#include "K2Node_CallFunction.h"
#include "KismetCompiler.h"
#include "BlueprintTool/ExParallelProxy.h"

#define LOCTEXT_NAMESPACE "K2Node_Parallel"

UExK2Node_Parallel::UExK2Node_Parallel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExParallelProxy, CreateProxy_All);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExParallelProxy, Activate);
	ProxyFactoryClass = UExParallelProxy::StaticClass();
	ProxyClass = UExParallelProxy::StaticClass();
}

void UExK2Node_Parallel::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	UEdGraphPin* CompletedPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Completed"));
	CompletedPin->PinFriendlyName = NSLOCTEXT("K2Node", "OutputCompleted", "Completed");
}

FText UExK2Node_Parallel::GetTooltipText() const
{
	return NSLOCTEXT("K2Node", "Parallel_Tooltip", "Execute multiple branches in parallel and trigger Completed when conditions are met");
}

FText UExK2Node_Parallel::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("K2Node", "Parallel_Title", "Parallel");
}

FSlateIcon UExK2Node_Parallel::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = FColor(200, 150, 100);
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Sequence_16x");
	return Icon;
}

void UExK2Node_Parallel::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);
}

UEdGraphPin* UExK2Node_Parallel::GetCompletedPin() const
{
	UEdGraphPin* Pin = FindPin(TEXT("Completed"));
	check(Pin == nullptr || Pin->Direction == EGPD_Output);
	return Pin;
}

#undef LOCTEXT_NAMESPACE
