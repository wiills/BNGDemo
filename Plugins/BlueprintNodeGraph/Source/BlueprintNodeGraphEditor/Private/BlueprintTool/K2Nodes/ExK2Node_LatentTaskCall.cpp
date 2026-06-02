// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/K2Nodes/ExK2Node_LatentTaskCall.h"

#include "EdGraphSchema_K2.h"
#include "Kismet/KismetSystemLibrary.h"
#include "K2Node_CallFunction.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintNodeSpawner.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "BlueprintActionDatabaseRegistrar.h"

#define LOCTEXT_NAMESPACE "UExK2Node_LatentTaskCall"

TArray<TWeakObjectPtr<UClass> > UExK2Node_LatentTaskCall::NodeClasses;

UExK2Node_LatentTaskCall::UExK2Node_LatentTaskCall(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExBase_AsyncAction, Activate);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExBase_AsyncAction, SetK2NodeInfo);
}

FText UExK2Node_LatentTaskCall::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (!NodeInfo.NodeName.IsEmpty())
	{
		return FText::FromString(NodeInfo.NodeName);
	}
	return LOCTEXT("LatentTaskCallNodeTitle", "Latent Task");
}

void UExK2Node_LatentTaskCall::RegisterSpecializedTaskNodeClass(TSubclassOf<UExK2Node_LatentTaskCall> NodeClass)
{
	if (NodeClass)
	{
		NodeClasses.AddUnique(*NodeClass);
	}
}

bool UExK2Node_LatentTaskCall::HasDedicatedNodeClass(TSubclassOf<UExBase_AsyncAction> TaskClass) 
{
	for (const TWeakObjectPtr<UClass>& NodeClass : NodeClasses)
	{
		if (NodeClass.IsValid())
		{
			const UExK2Node_LatentTaskCall* NodeCDO = NodeClass->GetDefaultObject<UExK2Node_LatentTaskCall>();
			if (NodeCDO && NodeCDO->IsHandling(TaskClass))
			{
				return true;
			}
		}
	}

	return false;
}

bool UExK2Node_LatentTaskCall::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const
{
	return Super::CanCreateUnderSpecifiedSchema(DesiredSchema);
}

void UExK2Node_LatentTaskCall::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	struct GetMenuActions_Utils
	{
		static void SetNodeFunc(UEdGraphNode* NewNode, bool /*bIsTemplateNode*/, TWeakObjectPtr<UFunction> FunctionPtr)
		{
			UExK2Node_LatentTaskCall* AsyncTaskNode = CastChecked<UExK2Node_LatentTaskCall>(NewNode);
			if (FunctionPtr.IsValid())
			{
				UFunction* Func = FunctionPtr.Get();
				FObjectProperty* ReturnProp = CastFieldChecked<FObjectProperty>(Func->GetReturnProperty());
						
				AsyncTaskNode->ProxyFactoryFunctionName = Func->GetFName();
				AsyncTaskNode->ProxyFactoryClass        = Func->GetOuterUClass();
				AsyncTaskNode->ProxyClass               = ReturnProp->PropertyClass;
			}
		}
	};

	UClass* NodeClass = GetClass();
	ActionRegistrar.RegisterClassFactoryActions<UExBase_AsyncAction>( FBlueprintActionDatabaseRegistrar::FMakeFuncSpawnerDelegate::CreateLambda([NodeClass](const UFunction* FactoryFunc)->UBlueprintNodeSpawner*
	{
		UBlueprintNodeSpawner* NodeSpawner = nullptr;
		
		UClass* FuncClass = FactoryFunc->GetOwnerClass();
		if (!HasDedicatedNodeClass(FuncClass))
		{
			NodeSpawner = UBlueprintFunctionNodeSpawner::Create(FactoryFunc);
			check(NodeSpawner != nullptr);
			NodeSpawner->NodeClass = NodeClass;
	
			TWeakObjectPtr<UFunction> FunctionPtr = MakeWeakObjectPtr(const_cast<UFunction*>(FactoryFunc));
			NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(GetMenuActions_Utils::SetNodeFunc, FunctionPtr);
		}
		return NodeSpawner;
	}) );
}

FSlateIcon UExK2Node_LatentTaskCall::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Default_16x");
	return Icon;
}

UEdGraphPin* UExK2Node_LatentTaskCall::GetResultPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

bool UExK2Node_LatentTaskCall::ValidateClassPin(FKismetCompilerContext& CompilerContext, bool bGenerateErrors)
{
	const bool HasClassParameter = GetClassPin() != nullptr;
	if (!HasClassParameter)
	{
		if (bGenerateErrors)
		{
			CompilerContext.MessageLog.Error(*LOCTEXT("MissingClassParameter", "UExK2Node_LatentTaskCall: Proxy Factory Function missing a Class parameter. @@").ToString(), this);
		}
		return false;
	}
	return true;
}

void UExK2Node_LatentTaskCall::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
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
	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(UEdGraphSchema_K2::PN_Execute), *CallCreateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute)).CanSafeConnect();
	for (UEdGraphPin* CurrentPin : Pins)
	{
		if (FBaseAsyncTaskHelper::ValidDataPin(CurrentPin, EGPD_Input))
		{
			UEdGraphPin* DestPin = CallCreateProxyObjectNode->FindPin(CurrentPin->PinName);
			if (DestPin)
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
			bIsErrorFree &= TempVarOutput->GetVariablePin() && CompilerContext.MovePinLinksToIntermediate(*CurrentPin, *TempVarOutput->GetVariablePin()).CanSafeConnect();
			VariableOutputs.Add(FBaseAsyncTaskHelper::FOutputPinAndLocalVariable(CurrentPin, TempVarOutput));
		}
	}
	
	UEdGraphPin* LastThenPin = CallCreateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);

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
		CompilerContext.MessageLog.Error(*LOCTEXT("MissingDelegateProperties", "BaseAsyncTask: Proxy has no delegates defined. @@").ToString(), this);
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
		bIsErrorFree &= Schema->TryCreateConnection(ProxyActivateIsValidFuncNode->GetReturnValuePin(), ProxyActivateValidateProxyNode->GetConditionPin());

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
		CompilerContext.MessageLog.Error(*LOCTEXT("InternalConnectionError", "BaseAsyncTask: Internal connection error. @@").ToString(), this);
	}

	BreakAllNodeLinks();
}

#undef LOCTEXT_NAMESPACE
