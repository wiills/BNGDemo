// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/K2Nodes/ExK2Node_LatentTaskObject.h"

#include "EdGraphSchema_K2.h"
#include "Kismet/KismetSystemLibrary.h"
#include "K2Node_CallFunction.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "BlueprintTool/LatentTasks/ExLatentTask_Custom.h"

#define LOCTEXT_NAMESPACE "UExK2Node_LatentTaskObject"


// proxy class
using LatentTaskClassDefine = UExLatentTask_Custom;

UExK2Node_LatentTaskObject::UExK2Node_LatentTaskObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(LatentTaskClassDefine, CreateProxy);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(LatentTaskClassDefine, SetK2NodeInfo);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(LatentTaskClassDefine, Activate);
	ProxyFactoryClass = LatentTaskClassDefine::StaticClass();
	ProxyClass = LatentTaskClassDefine::StaticClass();
}

bool UExK2Node_LatentTaskObject::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const
{
	return Super::CanCreateUnderSpecifiedSchema(DesiredSchema);
}

void UExK2Node_LatentTaskObject::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	
	for (const UStruct* TestStruct = ProxyClass; TestStruct; TestStruct = TestStruct->GetSuperStruct())
	{
		const bool bSafeHideThen = TestStruct->HasMetaData(TEXT("SafeHideThen"));
		if (bSafeHideThen && GetThenPin())
		{
			GetThenPin()->SafeSetHidden(true);
			break;
		}
	}
}

FSlateIcon UExK2Node_LatentTaskObject::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Default_16x");
	return Icon;
}

// -------------------------------------------------

// struct FK2Node_LatentTaskObjectHelper
// {
// 	static FName WorldContextPinName;
// 	static FName ClassPinName;
// };
//
// FName FK2Node_LatentTaskObjectHelper::WorldContextPinName(TEXT("WorldContextObject"));
// FName FK2Node_LatentTaskObjectHelper::ClassPinName(TEXT("Class"));

// -------------------------------------------------

void UExK2Node_LatentTaskObject::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();
	UClass* UseSpawnClass = GetClassToSpawn(&OldPins);
	if (UseSpawnClass != nullptr)
	{
		CreatePinsForClass(UseSpawnClass);
	}
	RestoreSplitPins(OldPins);
}

UEdGraphPin* UExK2Node_LatentTaskObject::GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch /*= NULL*/) const
{
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* Pin = nullptr;
	for (UEdGraphPin* TestPin : *PinsToSearch)
	{
		if (TestPin && TestPin->PinName == ExLatentTaskHelper::ClassPinName)
		{
			Pin = TestPin;
			break;
		}
	}
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

UClass* UExK2Node_LatentTaskObject::GetClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch) const
{
	UClass* UseSpawnClass = nullptr;
	const TArray<UEdGraphPin*>* PinsToSearch = InPinsToSearch ? InPinsToSearch : &Pins;

	UEdGraphPin* ClassPin = GetClassPin(PinsToSearch);
	if (ClassPin && ClassPin->DefaultObject != nullptr && ClassPin->LinkedTo.Num() == 0)
	{
		UseSpawnClass = CastChecked<UClass>(ClassPin->DefaultObject);
	}
	else if (ClassPin && (1 == ClassPin->LinkedTo.Num()))
	{
		UEdGraphPin* SourcePin = ClassPin->LinkedTo[0];
		UseSpawnClass = SourcePin ? Cast<UClass>(SourcePin->PinType.PinSubCategoryObject.Get()) : nullptr;
	}

	return UseSpawnClass;
}

void UExK2Node_LatentTaskObject::CreatePinsForClass(UClass* InClass)
{
	check(InClass != nullptr);

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

	const UObject* const ClassDefaultObject = InClass->GetDefaultObject(false);

	SpawnParamPins.Reset();

	// Tasks can hide spawn parameters by doing meta = (HideSpawnParms="PropertyA,PropertyB")
	// (For example, hide Instigator in situations where instigator is not relevant to your task)
	
	TArray<FString> IgnorePropertyList;
	{
		const UFunction* ProxyFunction = ProxyFactoryClass->FindFunctionByName(ProxyFactoryFunctionName);

		const FString& IgnorePropertyListStr = ProxyFunction->GetMetaData(FName(TEXT("HideSpawnParms")));
	
		if (!IgnorePropertyListStr.IsEmpty())
		{
			IgnorePropertyListStr.ParseIntoArray(IgnorePropertyList, TEXT(","), true);
		}
	}

	for (TFieldIterator<FProperty> PropertyIt(InClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		const FProperty* Property = *PropertyIt;
		UClass* PropertyClass = Property->GetOwnerChecked<UClass>();
		const bool bIsDelegate = Property->IsA(FMulticastDelegateProperty::StaticClass());
		const bool bIsExposedToSpawn = UEdGraphSchema_K2::IsPropertyExposedOnSpawn(Property);
		const bool bIsSettableExternally = !Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance);

		if (bIsExposedToSpawn &&
			!Property->HasAnyPropertyFlags(CPF_Parm) &&
			bIsSettableExternally &&
			Property->HasAllPropertyFlags(CPF_BlueprintVisible) &&
			!bIsDelegate && 
			!IgnorePropertyList.Contains(Property->GetName()) &&
			(FindPin(Property->GetFName()) == nullptr) )
		{
			UEdGraphPin* Pin = CreatePin(EGPD_Input, NAME_None, Property->GetFName());
			check(Pin);
			const bool bPinGood = K2Schema->ConvertPropertyToPinType(Property, /*out*/ Pin->PinType);
			SpawnParamPins.Add(Pin->PinName);

			if (ClassDefaultObject && K2Schema->PinDefaultValueIsEditable(*Pin))
			{
				FString DefaultValueAsString;
				const bool bDefaultValueSet = FBlueprintEditorUtils::PropertyValueToString(Property, reinterpret_cast<const uint8*>(ClassDefaultObject), DefaultValueAsString, this);
				check(bDefaultValueSet);
				K2Schema->SetPinAutogeneratedDefaultValue(Pin, DefaultValueAsString);
			}

			// Copy tooltip from the property.
			K2Schema->ConstructBasicPinTooltip(*Pin, Property->GetToolTipText(), Pin->PinToolTip);
		}
	}
}

void UExK2Node_LatentTaskObject::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin->PinName == ExLatentTaskHelper::ClassPinName)
	{
		const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();

		// Track removed pins so that we can reconnect it later if possible
		TArray<UEdGraphPin*> RemovedPins;

		// Orphan all pins related to archetype variables that have connections, otherwise just remove them
		for (const FName& OldPinReference : SpawnParamPins)
		{
			if(UEdGraphPin* OldPin = FindPin(OldPinReference))
			{
				if(OldPin->HasAnyConnections())
				{
					RemovedPins.Add(OldPin);
				}
				Pins.Remove(OldPin);
			}
		}
		
		SpawnParamPins.Reset();

		UClass* UseSpawnClass = GetClassToSpawn();
		if (UseSpawnClass != nullptr)
		{
			CreatePinsForClass(UseSpawnClass);
		}

		RewireOldPinsToNewPins(/* InOldPins = */ RemovedPins, /* InNewPins = */ Pins, /* NewPinToOldPin = */ nullptr);

		// Refresh the UI for the graph so the pin changes show up
		UEdGraph* Graph = GetGraph();
		Graph->NotifyGraphChanged();

		// Mark dirty
		FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
	}
}

UEdGraphPin* UExK2Node_LatentTaskObject::GetResultPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UExK2Node_LatentTaskObject::GetThenPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_Then);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

/**
 *	This is essentially a mix of K2Node_BaseAsyncTask::ExpandNode and K2Node_SpawnActorFromClass::ExpandNode and K2Node_GenericCreateObject::ExpandNode.
 *	Several things are going on here:
 *		-Factory call to create proxy object (K2Node_BaseAsyncTask)
 *		-Task return delegates are created and hooked up (K2Node_BaseAsyncTask)
 *	
 */
void UExK2Node_LatentTaskObject::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	check(SourceGraph && Schema);
	
	// no select special class
	const UEdGraphPin* ClassPin = GetClassPin();
	if (ClassPin == nullptr)
	{
		// Nothing special about this task, just call super
		Super::ExpandNode(CompilerContext, SourceGraph);
		return;
	}

	// SetValue: UUID && InputCount
	SetUUIDAndNodeInfo(Schema);
	
	
	UK2Node::ExpandNode(CompilerContext, SourceGraph);
	
	bool bIsErrorFree = true;

	// ------------------------------------------------------------------------------------------
	// CREATE A CALL TO FACTORY THE PROXY OBJECT
	// ------------------------------------------------------------------------------------------
	UK2Node_CallFunction* const CallCreateProxyObjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallCreateProxyObjectNode->FunctionReference.SetExternalMember(ProxyFactoryFunctionName, ProxyFactoryClass);
	CallCreateProxyObjectNode->AllocateDefaultPins();
	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(UEdGraphSchema_K2::PN_Execute), *CallCreateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute)).CanSafeConnect();
	for (UEdGraphPin* CurrentPin : Pins)
	{
		if (FBaseAsyncTaskHelper::ValidDataPin(CurrentPin, EGPD_Input))
		{
			UEdGraphPin* DestPin = CallCreateProxyObjectNode->FindPin(CurrentPin->PinName); // match function inputs, to pass data to function from CallFunction node

			// NEW: if no DestPin, assume it is a Class Spawn PRoperty - not an error
			if (DestPin)
			{
				bIsErrorFree &= CompilerContext.CopyPinLinksToIntermediate(*CurrentPin, *DestPin).CanSafeConnect();
			}
		}
	}

	// Expose Async Task Proxy object
	UEdGraphPin* const ProxyObjectPin = CallCreateProxyObjectNode->GetReturnValuePin();
	check(ProxyObjectPin);
	UEdGraphPin* OutputAsyncTaskProxy = FindPin(FBaseAsyncTaskHelper::GetAsyncTaskProxyName());
	if (OutputAsyncTaskProxy)
	{
		bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OutputAsyncTaskProxy, *ProxyObjectPin).CanSafeConnect();
	}
	
	// ------------------------------------------------------------------------------------------
	// GATHER OUTPUT PARAMETERS AND PAIR THEM WITH LOCAL VARIABLES
	// ------------------------------------------------------------------------------------------
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
	
	// ------------------------------------------------------------------------------------------
	// ASSIGN TARGET_CLASS PROPERTY PIN VARIABLES TO NEW TARGET_OBJECT
	// ------------------------------------------------------------------------------------------
	UEdGraphPin* LastThenPin;
	// assign exposed values and connect then
	{
		const auto TargetClass = GetClassToSpawn();
		LastThenPin = FKismetCompilerUtilities::GenerateAssignmentNodes(CompilerContext, SourceGraph, CallCreateProxyObjectNode, this, ProxyObjectPin, TargetClass);
		UEdGraphPin* SpawnNodeThen = GetThenPin();
		bIsErrorFree &= SpawnNodeThen && LastThenPin && CompilerContext.MovePinLinksToIntermediate(*SpawnNodeThen, *LastThenPin).CanSafeConnect();
	}
	
	// ------------------------------------------------------------------------------------------
	// FOR EACH DELEGATE DEFINE EVENT, CONNECT IT TO DELEGATE AND IMPLEMENT A CHAIN OF ASSIGMENTS
	// ------------------------------------------------------------------------------------------

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

	// handle delegates
	bIsErrorFree &= HandleDelegates(VariableOutputs, ProxyObjectPin, LastThenPin, SourceGraph, CompilerContext);

	if (CallCreateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Then) == LastThenPin)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("MissingDelegateProperties", "BaseAsyncTask: Proxy has no delegates defined. @@").ToString(), this);
		return;
	}
	
	// --------------------------------------------------------------------------------------
	// Create call function "SetK2NodeInfo" to set info to proxy object if necessary
	// --------------------------------------------------------------------------------------
	UK2Node_IfThenElse* ProxySetK2NodeInfoValidateNode = nullptr;
	if (ProxySetK2NodeInfoFunctionName != NAME_None)
	{
		CompilerSetK2NodeInfoCall(CompilerContext, SourceGraph, ProxySetK2NodeInfoValidateNode, ProxyObjectPin, LastThenPin, bIsErrorFree);
	}
	
	// --------------------------------------------------------------------------------------
	// Create a call to activate the proxy object if necessary
	// --------------------------------------------------------------------------------------

	UK2Node_IfThenElse* ProxyActivateValidateProxyNode = nullptr;

	if (ProxyActivateFunctionName != NAME_None)
	{
		// Validate the proxy object is still valid. Its possible the task ends while calling FinishSpawning, in which case we don't need to call the ProxyActivateFunction.		
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

		// Actually call the Activate function
		UK2Node_CallFunction* const CallActivateProxyObjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		CallActivateProxyObjectNode->FunctionReference.SetExternalMember(ProxyActivateFunctionName, ProxyClass);
		CallActivateProxyObjectNode->AllocateDefaultPins();

		// Hook up the self connection
		UEdGraphPin* ActivateCallSelfPin = Schema->FindSelfPin(*CallActivateProxyObjectNode, EGPD_Input);
		check(ActivateCallSelfPin);

		bIsErrorFree &= Schema->TryCreateConnection(ProxyObjectPin, ActivateCallSelfPin);

		// Hook the activate node up in the exec chain
		UEdGraphPin* ActivateExecPin = CallActivateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute);
		UEdGraphPin* ActivateThenPin = CallActivateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);

		bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, ActivateExecPin);

		LastThenPin = ActivateThenPin;
	}

	// --------------------------------------------------------------------------------------
	// Move the connections from the original node then pin to the last internal then pin
	// --------------------------------------------------------------------------------------

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

	// Make sure we caught everything
	BreakAllNodeLinks();
}


#undef LOCTEXT_NAMESPACE
