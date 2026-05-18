// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExK2Node_Parallel.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintCompiledStatement.h"
#include "BlueprintNodeSpawner.h"
#include "Containers/UnrealString.h"
#include "EdGraphSchema_K2.h"
#include "Internationalization/Internationalization.h"
#include "K2Node_CallFunction.h"
#include "Kismet2/CompilerResultsLog.h"
#include "KismetCompiledFunctionContext.h"
#include "KismetCompiler.h"
#include "KismetCompilerMisc.h"
#include "Misc/AssertionMacros.h"
#include "Styling/AppStyle.h"
#include "Templates/Casts.h"

#define LOCTEXT_NAMESPACE "ExK2Node_Parallel"

// -----------------------------------------------------------------------------
// 编译行为复制自 Engine/BlueprintGraph/Private/K2Node_ExecutionSequence.cpp 中的
// FKCHandler_ExecutionSequence（该类在引擎 .cpp 内不可见）。Parallel 的 Then_* 命名与 Sequence 一致，
// 故生成语句与原生 Sequence 节点相同。
// -----------------------------------------------------------------------------
namespace ExParallelCompile
{
#define EX_HANDLER_LOCTEXT_NAMESPACE "ExParallelCompile_Handler"

class FKCHandler_Parallel : public FNodeHandlingFunctor
{
public:
	explicit FKCHandler_Parallel(FKismetCompilerContext& InCompilerContext)
		: FNodeHandlingFunctor(InCompilerContext)
	{
	}

	virtual void Compile(FKismetFunctionContext& Context, UEdGraphNode* Node) override
	{
		FEdGraphPinType ExpectedPinType;
		ExpectedPinType.PinCategory = UEdGraphSchema_K2::PC_Exec;

		UEdGraphPin* ExecTriggeringPin =
			Context.FindRequiredPinByName(Node, UEdGraphSchema_K2::PN_Execute, EGPD_Input);
		if ((ExecTriggeringPin == nullptr) || !Context.ValidatePinType(ExecTriggeringPin, ExpectedPinType))
		{
			CompilerContext.MessageLog.Error(
				*NSLOCTEXT(EX_HANDLER_LOCTEXT_NAMESPACE, "NoValidExecutionPinForExecSeq_Error",
					"@@ must have a valid execution pin @@")
					 .ToString(),
				Node, ExecTriggeringPin);
			return;
		}
		else if (ExecTriggeringPin->LinkedTo.Num() == 0)
		{
			CompilerContext.MessageLog.Warning(
				*NSLOCTEXT(EX_HANDLER_LOCTEXT_NAMESPACE, "NodeNeverExecuted_Warning", "@@ will never be executed")
					 .ToString(),
				Node);
			return;
		}

		TArray<UEdGraphPin*> OutputPins;
		for (UEdGraphPin* CurrentPin : Node->Pins)
		{
			if ((CurrentPin->Direction == EGPD_Output) && (CurrentPin->LinkedTo.Num() > 0) &&
				(CurrentPin->PinName.ToString().StartsWith(UEdGraphSchema_K2::PN_Then.ToString())))
			{
				OutputPins.Add(CurrentPin);
			}
		}

		if (OutputPins.Num() > 0)
		{
			if (Context.IsDebuggingOrInstrumentationRequired() && (OutputPins.Num() > 1))
			{
				const FString NodeComment = Node->NodeComment.IsEmpty() ? Node->GetName() : Node->NodeComment;

				FBlueprintCompiledStatement* LastPushStatement = nullptr;

				for (int32 i = 0; i < OutputPins.Num(); ++i)
				{
					const bool bNotFirstIndex = i > 0;
					if (bNotFirstIndex)
					{
						FBlueprintCompiledStatement& DebugSiteAndJumpTarget = Context.AppendStatementForNode(Node);
						DebugSiteAndJumpTarget.Type = Context.GetBreakpointType();
						DebugSiteAndJumpTarget.Comment = NodeComment;
						DebugSiteAndJumpTarget.bIsJumpTarget = true;

						check(LastPushStatement);
						LastPushStatement->TargetLabel = &DebugSiteAndJumpTarget;
					}

					const bool bNotLastIndex = ((i + 1) < OutputPins.Num());
					if (bNotLastIndex)
					{
						FBlueprintCompiledStatement& PushExecutionState = Context.AppendStatementForNode(Node);
						PushExecutionState.Type = KCST_PushState;
						LastPushStatement = &PushExecutionState;
					}

					FBlueprintCompiledStatement& GotoSequenceLinkedState = Context.AppendStatementForNode(Node);
					GotoSequenceLinkedState.Type = KCST_UnconditionalGoto;
					Context.GotoFixupRequestMap.Add(&GotoSequenceLinkedState, OutputPins[i]);
				}

				check(LastPushStatement);
			}
			else
			{
				for (int32 i = OutputPins.Num() - 1; i > 0; i--)
				{
					FBlueprintCompiledStatement& PushExecutionState = Context.AppendStatementForNode(Node);
					PushExecutionState.Type = KCST_PushState;
					Context.GotoFixupRequestMap.Add(&PushExecutionState, OutputPins[i]);
				}

				FBlueprintCompiledStatement& NextExecutionState = Context.AppendStatementForNode(Node);
				NextExecutionState.Type = KCST_UnconditionalGoto;
				Context.GotoFixupRequestMap.Add(&NextExecutionState, OutputPins[0]);
			}
		}
		else
		{
			FBlueprintCompiledStatement& NextExecutionState = Context.AppendStatementForNode(Node);
			NextExecutionState.Type = KCST_EndOfThread;
		}
	}
};

#undef EX_HANDLER_LOCTEXT_NAMESPACE

}

static const FName GParallelOutputCountPin(TEXT("OutputCount"));

UExK2Node_Parallel::UExK2Node_Parallel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UExK2Node_Parallel::EnsureOutputCountPin()
{
	if (FindPin(GParallelOutputCountPin))
	{
		return;
	}

	UEdGraphPin* CP = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Int, GParallelOutputCountPin);
	CP->PinFriendlyName = NSLOCTEXT("ExK2Node_Parallel", "CountPinFriendly", "Count");
	CP->DefaultValue = TEXT("2");
}

int32 UExK2Node_Parallel::CountThenExecOutputs() const
{
	int32 C = 0;
	for (UEdGraphPin* P : Pins)
	{
		if (P && P->Direction == EGPD_Output && UEdGraphSchema_K2::IsExecPin(*P))
		{
			++C;
		}
	}
	return C;
}

int32 UExK2Node_Parallel::ResolveOutputCountFromCountPin(const UEdGraphPin* CountPin)
{
	static constexpr int32 MinC = 2;
	static constexpr int32 MaxC = 32;
	if (!CountPin)
	{
		return MinC;
	}

	int32 Parsed = FCString::Atoi(*CountPin->DefaultValue);

	if (CountPin->LinkedTo.Num() == 1)
	{
		const UEdGraphPin* SrcOut = CountPin->LinkedTo[0];
		if (SrcOut && SrcOut->Direction == EGPD_Output)
		{
			if (SrcOut->PinType.PinCategory == UEdGraphSchema_K2::PC_Int && !SrcOut->DefaultValue.IsEmpty())
			{
				Parsed = FCString::Atoi(*SrcOut->DefaultValue);
			}
			else if (const UK2Node_CallFunction* Fn = Cast<UK2Node_CallFunction>(SrcOut->GetOwningNode()))
			{
				auto TryReadIntPinDefault = [&](const FName PinName) -> bool
				{
					const UEdGraphPin* VP = Fn->FindPin(PinName);
					if (VP && VP->PinType.PinCategory == UEdGraphSchema_K2::PC_Int && !VP->DefaultValue.IsEmpty())
					{
						Parsed = FCString::Atoi(*VP->DefaultValue);
						return true;
					}
					return false;
				};
				static const FName CandidatePinNames[] = {TEXT("Value"), TEXT("NewValue")};
				for (const FName N : CandidatePinNames)
				{
					if (TryReadIntPinDefault(N))
					{
						break;
					}
				}
			}
		}
	}

	if (Parsed < MinC)
	{
		Parsed = MinC;
	}
	return FMath::Clamp(Parsed, MinC, MaxC);
}

int32 UExK2Node_Parallel::GetOutputCountDefault() const
{
	return ResolveOutputCountFromCountPin(FindPin(GParallelOutputCountPin));
}

void UExK2Node_Parallel::SetCountPinDefaultFromThenCount()
{
	if (UEdGraphPin* C = FindPin(GParallelOutputCountPin))
	{
		C->DefaultValue = FString::FromInt(CountThenExecOutputs());
	}
}

FName UExK2Node_Parallel::GetPinNameGivenIndex(const int32 Index) const
{
	return *FString::Printf(TEXT("%s_%d"), *UEdGraphSchema_K2::PN_Then.ToString(), Index);
}

FName UExK2Node_Parallel::GetUniquePinName()
{
	FName NewPinName;
	int32 i = 0;
	while (true)
	{
		NewPinName = GetPinNameGivenIndex(i++);
		if (!FindPin(NewPinName))
		{
			break;
		}
	}
	return NewPinName;
}

UEdGraphPin* UExK2Node_Parallel::GetThenPinGivenIndex(const int32 Index)
{
	return FindPin(GetPinNameGivenIndex(Index));
}

void UExK2Node_Parallel::RemovePinFromExecutionNode(UEdGraphPin* TargetPin)
{
	if (UExK2Node_Parallel* OwningSeq = Cast<UExK2Node_Parallel>(TargetPin->GetOwningNode()))
	{
		OwningSeq->Pins.Remove(TargetPin);
		TargetPin->MarkAsGarbage();

		int32 ThenIndex = 0;
		for (int32 i = 0; i < OwningSeq->Pins.Num(); ++i)
		{
			UEdGraphPin* PotentialPin = OwningSeq->Pins[i];
			if (UEdGraphSchema_K2::IsExecPin(*PotentialPin) && (PotentialPin->Direction == EGPD_Output))
			{
				PotentialPin->PinName = GetPinNameGivenIndex(ThenIndex);
				++ThenIndex;
			}
		}
	}
}

bool UExK2Node_Parallel::CanRemoveExecutionPin() const
{
	int32 NumOutPins = 0;
	for (int32 i = 0; i < Pins.Num(); ++i)
	{
		const UEdGraphPin* PotentialPin = Pins[i];
		if (UEdGraphSchema_K2::IsExecPin(*PotentialPin) && (PotentialPin->Direction == EGPD_Output))
		{
			NumOutPins++;
		}
	}
	return (NumOutPins > 2);
}

void UExK2Node_Parallel::AddThenOutputPin()
{
	Modify();
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, GetUniquePinName());
}

void UExK2Node_Parallel::SyncThenOutputsToCountPin()
{
	const int32 Desired = GetOutputCountDefault();
	int32 Num = CountThenExecOutputs();

	while (Num < Desired)
	{
		AddThenOutputPin();
		Num = CountThenExecOutputs();
	}

	while (Num > Desired && CanRemoveExecutionPin())
	{
		UEdGraphPin* LastThen = GetThenPinGivenIndex(Num - 1);
		if (LastThen)
		{
			RemovePinFromExecutionNode(LastThen);
		}
		Num = CountThenExecOutputs();
	}

	SetCountPinDefaultFromThenCount();
}

void UExK2Node_Parallel::AddInputPin()
{
	AddThenOutputPin();
	SetCountPinDefaultFromThenCount();
}

void UExK2Node_Parallel::AllocateDefaultPins()
{
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, GetPinNameGivenIndex(0));
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, GetPinNameGivenIndex(1));
	Super::AllocateDefaultPins();
	EnsureOutputCountPin();
	SyncThenOutputsToCountPin();
}

void UExK2Node_Parallel::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	FString SavedCount(TEXT("2"));
	for (UEdGraphPin* Old : OldPins)
	{
		if (Old && Old->PinName == GParallelOutputCountPin)
		{
			SavedCount = Old->DefaultValue;
			break;
		}
	}

	Super::AllocateDefaultPins();

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);

	int32 ExecOutPinCount = 0;
	for (int32 i = 0; i < OldPins.Num(); ++i)
	{
		UEdGraphPin* TestPin = OldPins[i];
		if (UEdGraphSchema_K2::IsExecPin(*TestPin) && (TestPin->Direction == EGPD_Output))
		{
			const FName NewPinName(GetPinNameGivenIndex(ExecOutPinCount));
			ExecOutPinCount++;
			TestPin->PinName = NewPinName;
			CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, NewPinName);
		}
	}

	EnsureOutputCountPin();
	if (UEdGraphPin* CP = FindPin(GParallelOutputCountPin))
	{
		CP->DefaultValue = SavedCount;
	}
	SyncThenOutputsToCountPin();
}

void UExK2Node_Parallel::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);
	if (Pin && Pin->PinName == GParallelOutputCountPin)
	{
		Modify();
		ReconstructNode();
	}
}

void UExK2Node_Parallel::NotifyPinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::NotifyPinConnectionListChanged(Pin);
	if (Pin && Pin->PinName == GParallelOutputCountPin)
	{
		Modify();
		ReconstructNode();
	}
}

FNodeHandlingFunctor* UExK2Node_Parallel::CreateNodeHandler(FKismetCompilerContext& CompilerContext) const
{
	return new ExParallelCompile::FKCHandler_Parallel(CompilerContext);
}

void UExK2Node_Parallel::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FLinearColor UExK2Node_Parallel::GetNodeTitleColor() const
{
	return FLinearColor::White;
}

FText UExK2Node_Parallel::GetTooltipText() const
{
	return NSLOCTEXT(
		"ExK2Node_Parallel", "ParallelFlowTip",
		"Then_0、Then_1… 顺序触发（与 Sequence 相同）；Count（整型）决定 Then 输出数量（2–32）。可直接填默认值，或接入 Make Literal Int / 常量输出引脚；连线变化会自动重构引脚。");
}

FText UExK2Node_Parallel::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return NSLOCTEXT("ExK2Node_Parallel", "ParallelTitle", "Parallel");
}

FSlateIcon UExK2Node_Parallel::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = FLinearColor(0.78f, 0.59f, 0.39f);
	static FSlateIcon Icon(FAppStyle::GetAppStyleSetName(), "GraphEditor.Sequence_16x");
	return Icon;
}

FText UExK2Node_Parallel::GetMenuCategory() const
{
	return NSLOCTEXT("ExK2Node_Parallel", "MenuCategory", "Blueprint Node Graph|Flow Control");
}

#undef LOCTEXT_NAMESPACE
