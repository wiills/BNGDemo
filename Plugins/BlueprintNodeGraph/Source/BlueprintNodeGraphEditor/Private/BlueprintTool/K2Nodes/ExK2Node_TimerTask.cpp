// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/K2Nodes/ExK2Node_TimerTask.h"

#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "Kismet/KismetSystemLibrary.h"
#include "K2Node_CallFunction.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintTool/LatentTasks/ExLatentTask_Timer.h"

#define LOCTEXT_NAMESPACE "UExK2Node_TimerTask"

UExK2Node_TimerTask::UExK2Node_TimerTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExLatentTask_Timer, CreateTimerProxy);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExLatentTask_Timer, SetK2NodeInfo);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExLatentTask_Timer, Activate);
	ProxyFactoryClass = UExLatentTask_Timer::StaticClass();
	ProxyClass = UExLatentTask_Timer::StaticClass();
}

FText UExK2Node_TimerTask::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (!NodeInfo.NodeName.IsEmpty())
	{
		return FText::FromString(NodeInfo.NodeName);
	}
	if (TitleType == ENodeTitleType::ListView || TitleType == ENodeTitleType::MenuTitle)
	{
		return LOCTEXT("TimerTaskNodeTitle", "Timer Task");
	}

	const UClass* SpawnClass = GetClassToSpawn();
	if (SpawnClass && SpawnClass != UExLatentTask_Timer::StaticClass())
	{
		return FText::Format(
			LOCTEXT("TimerTaskNodeTitleWithClass", "Timer Task ({0})"),
			SpawnClass->GetDisplayNameText());
	}

	return LOCTEXT("TimerTaskNodeTitle", "Timer Task");
}

FText UExK2Node_TimerTask::GetTooltipText() const
{
	return LOCTEXT("TimerTaskNodeTooltip",
		"Create a countdown latent task. Duration is configured on this node. "
		"Optional Class selects a UExLatentTask_Timer blueprint subclass.");
}

FText UExK2Node_TimerTask::GetMenuCategory() const
{
	return LOCTEXT("TimerNodeCategory", "Blueprint Node Graph|Latent Task");
}

void UExK2Node_TimerTask::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

bool UExK2Node_TimerTask::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const
{
	return Super::CanCreateUnderSpecifiedSchema(DesiredSchema);
}

void UExK2Node_TimerTask::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	if (UEdGraphPin* ClassPin = GetClassPin())
	{
		ClassPin->PinType.PinSubCategoryObject = UExLatentTask_Timer::StaticClass();
	}
}

void UExK2Node_TimerTask::PostPlacedNewNode()
{
	Super::PostPlacedNewNode();

	if (UEdGraphPin* ClassPin = GetClassPin())
	{
		ClassPin->DefaultObject = UExLatentTask_Timer::StaticClass();
	}

	if (UClass* UseSpawnClass = GetClassToSpawn())
	{
		CreatePinsForClass(UseSpawnClass);
	}
}

FSlateIcon UExK2Node_TimerTask::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Default_16x");
	return Icon;
}

UClass* UExK2Node_TimerTask::GetValidBaseClass() const
{
	return UExLatentTask_Timer::StaticClass();
}

UEdGraphPin* UExK2Node_TimerTask::GetResultPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

void UExK2Node_TimerTask::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	check(SourceGraph && Schema);

	const UEdGraphPin* ClassPin = GetClassPin();
	if (ClassPin == nullptr)
	{
		Super::ExpandNode(CompilerContext, SourceGraph);
		return;
	}

	SetUUIDAndNodeInfo(Schema);
	UK2Node::ExpandNode(CompilerContext, SourceGraph);

	bool bIsErrorFree = true;

	UK2Node_CallFunction* const CallCreateProxyObjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallCreateProxyObjectNode->FunctionReference.SetExternalMember(ProxyFactoryFunctionName, ProxyFactoryClass);
	CallCreateProxyObjectNode->AllocateDefaultPins();
	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(
		*FindPinChecked(UEdGraphSchema_K2::PN_Execute),
		*CallCreateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute)).CanSafeConnect();

	for (UEdGraphPin* CurrentPin : Pins)
	{
		if (FBaseAsyncTaskHelper::ValidDataPin(CurrentPin, EGPD_Input))
		{
			if (UEdGraphPin* DestPin = CallCreateProxyObjectNode->FindPin(CurrentPin->PinName))
			{
				bIsErrorFree &= CompilerContext.CopyPinLinksToIntermediate(*CurrentPin, *DestPin).CanSafeConnect();
			}
		}
	}

	UEdGraphPin* const ProxyObjectPin = CallCreateProxyObjectNode->GetReturnValuePin();
	check(ProxyObjectPin);
	UEdGraphPin* OutputAsyncTaskProxy = FindPin(FBaseAsyncTaskHelper::GetAsyncTaskProxyName());
	if (OutputAsyncTaskProxy)
	{
		bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OutputAsyncTaskProxy, *ProxyObjectPin).CanSafeConnect();
	}

	TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable> VariableOutputs;
	for (UEdGraphPin* CurrentPin : Pins)
	{
		if ((OutputAsyncTaskProxy != CurrentPin) && FBaseAsyncTaskHelper::ValidDataPin(CurrentPin, EGPD_Output))
		{
			const FEdGraphPinType& PinType = CurrentPin->PinType;
			UK2Node_TemporaryVariable* TempVarOutput = CompilerContext.SpawnInternalVariable(
				this, PinType.PinCategory, PinType.PinSubCategory, PinType.PinSubCategoryObject.Get(), PinType.ContainerType, PinType.PinValueType);
			bIsErrorFree &= TempVarOutput->GetVariablePin()
				&& CompilerContext.MovePinLinksToIntermediate(*CurrentPin, *TempVarOutput->GetVariablePin()).CanSafeConnect();
			VariableOutputs.Add(FBaseAsyncTaskHelper::FOutputPinAndLocalVariable(CurrentPin, TempVarOutput));
		}
	}

	UEdGraphPin* LastThenPin = nullptr;
	{
		const UClass* TargetClass = GetClassToSpawn();
		LastThenPin = FKismetCompilerUtilities::GenerateAssignmentNodes(
			CompilerContext, SourceGraph, CallCreateProxyObjectNode, this, ProxyObjectPin, TargetClass);
		if (UEdGraphPin* SpawnNodeThen = GetThenPin())
		{
			bIsErrorFree &= SpawnNodeThen && LastThenPin
				&& CompilerContext.MovePinLinksToIntermediate(*SpawnNodeThen, *LastThenPin).CanSafeConnect();
		}
	}

	UK2Node_CallFunction* IsValidFuncNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	const FName IsValidFuncName = GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, IsValid);
	IsValidFuncNode->FunctionReference.SetExternalMember(IsValidFuncName, UKismetSystemLibrary::StaticClass());
	IsValidFuncNode->AllocateDefaultPins();
	UEdGraphPin* IsValidInputPin = IsValidFuncNode->FindPinChecked(ExLatentTaskHelper::ObjectPinName);
	bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, IsValidInputPin);

	UK2Node_IfThenElse* ValidateProxyNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
	ValidateProxyNode->AllocateDefaultPins();
	bIsErrorFree &= Schema->TryCreateConnection(IsValidFuncNode->GetReturnValuePin(), ValidateProxyNode->GetConditionPin());
	bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, ValidateProxyNode->GetExecPin());
	LastThenPin = ValidateProxyNode->GetThenPin();

	bIsErrorFree &= HandleDelegates(VariableOutputs, ProxyObjectPin, LastThenPin, SourceGraph, CompilerContext);

	if (CallCreateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Then) == LastThenPin)
	{
		CompilerContext.MessageLog.Error(
			*LOCTEXT("MissingDelegateProperties", "TimerTask: Proxy has no delegates defined. @@").ToString(), this);
		return;
	}

	UK2Node_IfThenElse* ProxySetK2NodeInfoValidateNode = nullptr;
	if (ProxySetK2NodeInfoFunctionName != NAME_None)
	{
		CompilerSetK2NodeInfoCall(CompilerContext, SourceGraph, ProxySetK2NodeInfoValidateNode, ProxyObjectPin, LastThenPin, bIsErrorFree);
	}

	UK2Node_IfThenElse* ProxyActivateValidateProxyNode = nullptr;
	if (ProxyActivateFunctionName != NAME_None)
	{
		UK2Node_CallFunction* ProxyActivateIsValidFuncNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		ProxyActivateIsValidFuncNode->FunctionReference.SetExternalMember(IsValidFuncName, UKismetSystemLibrary::StaticClass());
		ProxyActivateIsValidFuncNode->AllocateDefaultPins();
		UEdGraphPin* ProxyActivateIsValidInputPin = ProxyActivateIsValidFuncNode->FindPinChecked(ExLatentTaskHelper::ObjectPinName);
		bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, ProxyActivateIsValidInputPin);

		ProxyActivateValidateProxyNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
		ProxyActivateValidateProxyNode->AllocateDefaultPins();
		bIsErrorFree &= Schema->TryCreateConnection(
			ProxyActivateIsValidFuncNode->GetReturnValuePin(), ProxyActivateValidateProxyNode->GetConditionPin());
		bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, ProxyActivateValidateProxyNode->GetExecPin());
		LastThenPin = ProxyActivateValidateProxyNode->GetThenPin();

		UK2Node_CallFunction* const CallActivateProxyObjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		CallActivateProxyObjectNode->FunctionReference.SetExternalMember(ProxyActivateFunctionName, ProxyClass);
		CallActivateProxyObjectNode->AllocateDefaultPins();

		UEdGraphPin* ActivateCallSelfPin = Schema->FindSelfPin(*CallActivateProxyObjectNode, EGPD_Input);
		check(ActivateCallSelfPin);
		bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, ActivateCallSelfPin);

		UEdGraphPin* ActivateExecPin = CallActivateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute);
		UEdGraphPin* ActivateThenPin = CallActivateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);
		bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, ActivateExecPin);
		LastThenPin = ActivateThenPin;
	}

	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(UEdGraphSchema_K2::PN_Then), *LastThenPin).CanSafeConnect();
	bIsErrorFree &= CompilerContext.CopyPinLinksToIntermediate(*LastThenPin, *ValidateProxyNode->GetElsePin()).CanSafeConnect();
	if (ProxySetK2NodeInfoValidateNode)
	{
		bIsErrorFree &= CompilerContext.CopyPinLinksToIntermediate(*LastThenPin, *ProxySetK2NodeInfoValidateNode->GetElsePin()).CanSafeConnect();
	}
	if (ProxyActivateValidateProxyNode)
	{
		bIsErrorFree &= CompilerContext.CopyPinLinksToIntermediate(*LastThenPin, *ProxyActivateValidateProxyNode->GetElsePin()).CanSafeConnect();
	}

	if (!bIsErrorFree)
	{
		CompilerContext.MessageLog.Error(
			*LOCTEXT("InternalConnectionError", "TimerTask: Internal connection error. @@").ToString(), this);
	}

	BreakAllNodeLinks();
}

#undef LOCTEXT_NAMESPACE
