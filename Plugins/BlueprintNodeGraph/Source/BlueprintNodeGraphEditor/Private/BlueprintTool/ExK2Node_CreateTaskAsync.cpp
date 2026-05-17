// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/ExK2Node_CreateTaskAsync.h"

#include "K2Node_AddDelegate.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "KismetCompiler.h"
#include "K2Node_CreateDelegate.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_Self.h"
#include "K2Node_TemporaryVariable.h"
#include "BlueprintTool/ExLatentTaskBase.h"
#include "Kismet/KismetSystemLibrary.h"


#define LOCTEXT_NAMESPACE "UExK2Node_CreateTaskAsync"
using ProxyLatentObject = UExLatentTaskBase;


const FName OwnerPinName(TEXT("Owner"));
const FName OuterPinName(TEXT("Owner"));
const FName ObjectClassPinName(TEXT("ObjectClass"));
const FName WorldContextParamName(TEXT("WorldContextObject"));

namespace ExK2Node_CreateTaskHelper
{
	static bool CanSpawnObjectOfClass(TSubclassOf<UObject> ObjectClass, bool bAllowAbstract)
	{
		// Initially include types that meet the basic requirements.
		// Note: CLASS_Deprecated is an inherited class flag, so any subclass of an explicitly-deprecated class also cannot be spawned.
		bool bCanSpawnObject = (nullptr != *ObjectClass)
			&& (bAllowAbstract || !ObjectClass->HasAnyClassFlags(CLASS_Abstract))
			&& !ObjectClass->HasAnyClassFlags(CLASS_Deprecated | CLASS_NewerVersionExists);

		// UObject is a special case where if we are allowing abstract we are going to allow it through even though it doesn't have BlueprintType on it
		if (bCanSpawnObject && (!bAllowAbstract || (*ObjectClass != UObject::StaticClass())))
		{
			static const FName BlueprintTypeName(TEXT("BlueprintType"));
			static const FName NotBlueprintTypeName(TEXT("NotBlueprintType"));
			static const FName DontUseGenericSpawnObjectName(TEXT("DontUseGenericSpawnObject"));

			auto IsClassAllowedLambda = [](const UClass* InClass)
			{
				return InClass != AActor::StaticClass()
					&& InClass != UActorComponent::StaticClass();
			};

			// Exclude all types in the initial set by default.
			bCanSpawnObject = false;
			const UClass* CurrentClass = ObjectClass;

			// Climb up the class hierarchy and look for "BlueprintType." If "NotBlueprintType" is seen first, or if the class is not allowed, then stop searching.
			while (!bCanSpawnObject && CurrentClass != nullptr && !CurrentClass->GetBoolMetaData(NotBlueprintTypeName) && IsClassAllowedLambda(CurrentClass))
			{
				// Include any type that either includes or inherits 'BlueprintType'
				bCanSpawnObject = CurrentClass->GetBoolMetaData(BlueprintTypeName);

				// Stop searching if we encounter 'BlueprintType' with 'DontUseGenericSpawnObject'
				if (bCanSpawnObject && CurrentClass->GetBoolMetaData(DontUseGenericSpawnObjectName))
				{
					bCanSpawnObject = false;
					break;
				}

				CurrentClass = CurrentClass->GetSuperClass();
			}

			// If we validated the given class, continue walking up the hierarchy to make sure we exclude it if it's an Actor or ActorComponent derivative.
			while (bCanSpawnObject && CurrentClass != nullptr)
			{
				bCanSpawnObject &= IsClassAllowedLambda(CurrentClass);

				CurrentClass = CurrentClass->GetSuperClass();
			}
		}

		return bCanSpawnObject;
	}
}

UExK2Node_CreateTaskAsync::UExK2Node_CreateTaskAsync()
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(ProxyLatentObject, CreateTask);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(ProxyLatentObject, Activate);
	ProxyFactoryClass = ProxyLatentObject::StaticClass();
	ProxyClass = ProxyLatentObject::StaticClass();
}

FText UExK2Node_CreateTaskAsync::GetTooltipText() const
{
	return FText::FromString(TEXT("LatentTask ToolTip"));
}

FText UExK2Node_CreateTaskAsync::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	// return FText::FromString(TEXT("AsyncLatentTask"));
	return FText::FromString(TEXT(""));
}

TSharedPtr<SGraphNode> UExK2Node_CreateTaskAsync::CreateVisualWidget()
{
	return Super::CreateVisualWidget();
}

FText UExK2Node_CreateTaskAsync::GetMenuCategory() const
{
	return Super::GetMenuCategory();
}

UClass* UExK2Node_CreateTaskAsync::GetClassPinBaseClass() const
{
	return ProxyClass;
}

void UExK2Node_CreateTaskAsync::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);
}

void UExK2Node_CreateTaskAsync::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	Super::PinDefaultValueChanged(Pin);

	ReconstructNode();
}

void UExK2Node_CreateTaskAsync::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	Super::ReallocatePinsDuringReconstruction(OldPins);
}

void UExK2Node_CreateTaskAsync::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();
	
	AllocateDelegatePins();

	// is hide then pin
	GetThenPin()->SafeSetHidden(IsHideThen());

	// return value pin
	UEdGraphPin* SpawnResultPin = GetResultPin();
	SpawnResultPin->PinFriendlyName = LOCTEXT("Gameplay","ReturnTask");
	
	// // owner pin
	// UEdGraphPin* OwnerPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, AActor::StaticClass(), OwnerPinName);
	// OwnerPin->bAdvancedView = true;
	// if (UseOuter())
	// {
	// 	GetOuterPin()->bAdvancedView = true;
	// }
	// if (ENodeAdvancedPins::NoPins == AdvancedPinDisplay)
	// {
	// 	AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
	// }
}

void UExK2Node_CreateTaskAsync::AllocateDelegatePins()
{
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	// delegates pins
	for (TFieldIterator<FProperty> PropertyIt(GetClassPinBaseClass()); PropertyIt; ++PropertyIt)
	{
		if (const FMulticastDelegateProperty* Property = CastField<FMulticastDelegateProperty>(*PropertyIt))
		{
			UEdGraphPin* ExecPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, Property->GetFName());
			ExecPin->PinToolTip = Property->GetToolTipText().ToString();
			ExecPin->PinFriendlyName = Property->GetDisplayNameText();

			// create Delegate Params pin
			if (const auto DelegateSignatureFunction = Property->SignatureFunction)
			{
				for (TFieldIterator<FProperty> PropIt(DelegateSignatureFunction); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
				{
					const FProperty* Param = *PropIt;
					const bool bIsFunctionInput = !Param->HasAnyPropertyFlags(CPF_OutParm) || Param->HasAnyPropertyFlags(CPF_ReferenceParm);
					if (bIsFunctionInput)
					{
						UEdGraphPin* Pin = CreatePin(EGPD_Output, NAME_None, Param->GetFName());
						K2Schema->ConvertPropertyToPinType(Param, /*out*/ Pin->PinType);

						Pin->PinToolTip = Param->GetToolTipText().ToString();
					}
				}
			}
		}
	}
}

UEdGraphPin* UExK2Node_CreateTaskAsync::GetOwnerPin() const
{
	UEdGraphPin* Pin = FindPin(OwnerPinName);
	check(Pin == nullptr || Pin->Direction == EGPD_Input);
	return Pin;
}

void UExK2Node_CreateTaskAsync::EarlyValidation(class FCompilerResultsLog& MessageLog) const
{
	Super::EarlyValidation(MessageLog);
	const UEdGraphPin* ClassPin = GetClassPin(&Pins);
	const bool bAllowAbstract = ClassPin && ClassPin->LinkedTo.Num();
	UClass* ClassToSpawn = GetClassToSpawn();
	if (!ExK2Node_CreateTaskHelper::CanSpawnObjectOfClass(ClassToSpawn, bAllowAbstract))
	{
		MessageLog.Error(*FText::Format(LOCTEXT("CreateTaskAsync_WrongClassFmt", "Input Class Is '{0}' in @@"), FText::FromString(GetPathNameSafe(ClassToSpawn))).ToString(), this);
	}
}

bool UExK2Node_CreateTaskAsync::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	return UK2Node::IsCompatibleWithGraph(TargetGraph);
}

void UExK2Node_CreateTaskAsync::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	check(SourceGraph && Schema);
	
	const auto SpawnNode = this;
	UEdGraphPin* SpawnNodeExec = SpawnNode->GetExecPin();
	UEdGraphPin* SpawnWorldContextPin = SpawnNode->GetWorldContextPin();
	UEdGraphPin* SpawnNodeOwnerPin = SpawnNode->GetOwnerPin();
	
	UK2Node_CallFunction* CallCreateNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	CallCreateNode->FunctionReference.SetExternalMember(ProxyFactoryFunctionName, ProxyClass);
	CallCreateNode->AllocateDefaultPins();

	// call node pins
	UEdGraphPin* CallBeginWorldContextPin = CallCreateNode->FindPinChecked(WorldContextParamName);
	UEdGraphPin* CallBeginOwnerPin = CallCreateNode->FindPinChecked(OwnerPinName);
	UEdGraphPin* CallBeginExec = CallCreateNode->GetExecPin();
	
	// Move 'exec' connection from spawn node to 'begin spawn'
	CompilerContext.MovePinLinksToIntermediate(*SpawnNodeExec, *CallBeginExec);

	
	// store off the class to spawn before we mutate pin connections:
	const UClass* ClassToSpawn = GetClassToSpawn();
	
	bool bIsErrorFree = true;
	// connect exe
	{
		UEdGraphPin* SpawnExecPin = GetExecPin();
		UEdGraphPin* CallExecPin = CallCreateNode->GetExecPin();
		bIsErrorFree &= SpawnExecPin && CallExecPin && CompilerContext.MovePinLinksToIntermediate(*SpawnExecPin, *CallExecPin).CanSafeConnect();
	}

	//////////////////////// Context && Owner /////////////////////
	// Copy the world context connection from the spawn node to 'begin spawn' if necessary
	if (SpawnWorldContextPin)
	{
		CompilerContext.MovePinLinksToIntermediate(*SpawnWorldContextPin, *CallBeginWorldContextPin);
	}
	if (SpawnNodeOwnerPin != nullptr)
	{
		CompilerContext.MovePinLinksToIntermediate(*SpawnNodeOwnerPin, *CallBeginOwnerPin);
	}
	
	// connect outer
	UEdGraphPin* SpawnOuterPin = GetOuterPin();
	if (SpawnOuterPin)
	{
		UEdGraphPin* CallOuterPin = CallCreateNode->FindPin(OuterPinName);
		if (SpawnWorldContextPin && !CallOuterPin)
		{
			bIsErrorFree &= SpawnOuterPin && CallOuterPin && CompilerContext.MovePinLinksToIntermediate(*SpawnWorldContextPin, *CallOuterPin).CanSafeConnect();
		}else{
			bIsErrorFree &= SpawnOuterPin && CallOuterPin && CompilerContext.MovePinLinksToIntermediate(*SpawnOuterPin, *CallOuterPin).CanSafeConnect();
		}
	}
	///////////////////////////////////////////////////////////////

	// connect class
	{
		UEdGraphPin* SpawnClassPin = GetClassPin();
		UEdGraphPin* CallClassPin = CallCreateNode->FindPin(ObjectClassPinName);
		bIsErrorFree &= SpawnClassPin && CallClassPin && CompilerContext.MovePinLinksToIntermediate(*SpawnClassPin, *CallClassPin).CanSafeConnect();
		if (!bIsErrorFree)
		{
			CompilerContext.MessageLog.Error(*LOCTEXT("CreateTaskAsync_Error", "ICE: Class error @@").ToString(), this);
		}
	}

	UEdGraphPin* ReturnValuePin;
	// connect result
	{
		UEdGraphPin* SpawnResultPin = GetResultPin();
		ReturnValuePin = CallCreateNode->GetReturnValuePin();

		// cast HACK. It should be safe. The only problem is native code generation.
		if (SpawnResultPin && ReturnValuePin)
		{
			ReturnValuePin->PinType = SpawnResultPin->PinType;
		}
		bIsErrorFree &= SpawnResultPin && ReturnValuePin && CompilerContext.MovePinLinksToIntermediate(*SpawnResultPin, *ReturnValuePin).CanSafeConnect();
	}
	
	UEdGraphPin* LastThenPin;
	// assign exposed values and connect then
	{
		LastThenPin = FKismetCompilerUtilities::GenerateAssignmentNodes(CompilerContext, SourceGraph, CallCreateNode, this, ReturnValuePin, ClassToSpawn);
		UEdGraphPin* SpawnNodeThen = GetThenPin();
		bIsErrorFree &= SpawnNodeThen && LastThenPin && CompilerContext.MovePinLinksToIntermediate(*SpawnNodeThen, *LastThenPin).CanSafeConnect();
	}

	//////////////////////// Handle Delegates /////////////////////
	//LastThenPin = CallCreateNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);
	{
		UK2Node_CallFunction* IsValidFuncNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		const FName IsValidFuncName = GET_FUNCTION_NAME_CHECKED(UKismetSystemLibrary, IsValid);
		IsValidFuncNode->FunctionReference.SetExternalMember(IsValidFuncName, UKismetSystemLibrary::StaticClass());
		IsValidFuncNode->AllocateDefaultPins();
		UEdGraphPin* IsValidInputPin = IsValidFuncNode->FindPinChecked(TEXT("Object"));
		
		bIsErrorFree &= Schema->TryCreateConnection(ReturnValuePin, IsValidInputPin);
		
		UK2Node_IfThenElse* ValidateProxyNode = CompilerContext.SpawnIntermediateNode<UK2Node_IfThenElse>(this, SourceGraph);
		ValidateProxyNode->AllocateDefaultPins();
		bIsErrorFree &= Schema->TryCreateConnection(IsValidFuncNode->GetReturnValuePin(), ValidateProxyNode->GetConditionPin());
		
		bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, ValidateProxyNode->GetExecPin());
		LastThenPin = ValidateProxyNode->GetThenPin();

		if (CallCreateNode->FindPinChecked(UEdGraphSchema_K2::PN_Then) == LastThenPin)
		{
			CompilerContext.MessageLog.Error(*LOCTEXT("MissingDelegateProperties", "LatentTask: Proxy has no delegates defined. @@").ToString(), this);
			return;
		}
		
		/**** Handle Delegates *****/
		// FOR EACH DELEGATE DEFINE EVENT, CONNECT IT TO DELEGATE AND IMPLEMENT A CHAIN OF ASSIGMENTS
		bIsErrorFree &= HandleDelegates(ReturnValuePin, LastThenPin, SourceGraph, CompilerContext);
		
		// Create a call to activate the proxy object if necessary
		if (ProxyActivateFunctionName != NAME_None)
		{
			UK2Node_CallFunction* const CallActivateProxyObjectNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
			CallActivateProxyObjectNode->FunctionReference.SetExternalMember(ProxyActivateFunctionName, ProxyClass);
			CallActivateProxyObjectNode->AllocateDefaultPins();
	
			// Hook up the self connection
			UEdGraphPin* ActivateCallSelfPin = Schema->FindSelfPin(*CallActivateProxyObjectNode, EGPD_Input);
			check(ActivateCallSelfPin);
	
			bIsErrorFree &= Schema->TryCreateConnection(ReturnValuePin, ActivateCallSelfPin);
	
			// Hook the activate node up in the exec chain
			UEdGraphPin* ActivateExecPin = CallActivateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute);
			UEdGraphPin* ActivateThenPin = CallActivateProxyObjectNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);
	
			bIsErrorFree &= Schema->TryCreateConnection(LastThenPin, ActivateExecPin);
	
			LastThenPin = ActivateThenPin;
		}
		
		// Move the connections from the original node then pin to the last internal then pin
		UEdGraphPin* OriginalThenPin = FindPin(UEdGraphSchema_K2::PN_Then);
		
		if (OriginalThenPin)
		{
			bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*OriginalThenPin, *LastThenPin).CanSafeConnect();
		}
		bIsErrorFree &= CompilerContext.CopyPinLinksToIntermediate(*LastThenPin, *ValidateProxyNode->GetElsePin()).CanSafeConnect();
	}
	///////////////////////////////////////////////////////////////
	
	if (!bIsErrorFree)
	{
		CompilerContext.MessageLog.Error(*LOCTEXT("CreateTaskAsync_Error", "ICE: CreateTaskAsync Object error @@").ToString(), this);
	}
	
	BreakAllNodeLinks();
}

bool UExK2Node_CreateTaskAsync::HandleDelegates(UEdGraphPin* ProxyObjectPin, UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext)
{
	bool bIsErrorFree = true;
	for (TFieldIterator<FMulticastDelegateProperty> PropertyIt(ProxyClass); PropertyIt && bIsErrorFree; ++PropertyIt)
	{
		// handle one delegate
		UEdGraphPin* LastActivatedThenPin = nullptr;
		bIsErrorFree &= FAsyncTaskHelper::HandleDelegateImplementation(*PropertyIt, ProxyObjectPin, InOutLastThenPin, LastActivatedThenPin, this, SourceGraph, CompilerContext);
	}
	return bIsErrorFree;
}

#pragma region FAsyncTaskHelper

FName FAsyncTaskHelper::GetAsyncTaskProxyName()
{
	static const FName Name(TEXT("AsyncTaskProxy"));
	return Name;
}

bool FAsyncTaskHelper::ValidDataPin(const UEdGraphPin* Pin, EEdGraphPinDirection Direction)
{
	const bool bValidDataPin = Pin
		&& !Pin->bOrphanedPin
		&& (Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec);

	const bool bProperDirection = Pin && (Pin->Direction == Direction);

	return bValidDataPin && bProperDirection;
}

bool FAsyncTaskHelper::CreateDelegateForNewFunction(UEdGraphPin* DelegateInputPin, FName FunctionName, UK2Node* CurrentNode, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext)
{
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	check(DelegateInputPin && Schema && CurrentNode && SourceGraph && (FunctionName != NAME_None));
	bool bResult = true;

	// WORKAROUND, so we can create delegate from nonexistent function by avoiding check at expanding step
	// instead simply: Schema->TryCreateConnection(AddDelegateNode->GetDelegatePin(), CurrentCENode->FindPinChecked(UK2Node_CustomEvent::DelegateOutputName));
	UK2Node_Self* SelfNode = CompilerContext.SpawnIntermediateNode<UK2Node_Self>(CurrentNode, SourceGraph);
	SelfNode->AllocateDefaultPins();

	UK2Node_CreateDelegate* CreateDelegateNode = CompilerContext.SpawnIntermediateNode<UK2Node_CreateDelegate>(CurrentNode, SourceGraph);
	CreateDelegateNode->AllocateDefaultPins();
	bResult &= Schema->TryCreateConnection(DelegateInputPin, CreateDelegateNode->GetDelegateOutPin());
	bResult &= Schema->TryCreateConnection(SelfNode->FindPinChecked(UEdGraphSchema_K2::PN_Self), CreateDelegateNode->GetObjectInPin());
	CreateDelegateNode->SetFunction(FunctionName);

	return bResult;
}

bool FAsyncTaskHelper::CopyEventSignature(UK2Node_CustomEvent* CENode, UFunction* Function, const UEdGraphSchema_K2* Schema)
{
	check(CENode && Function && Schema);

	bool bResult = true;
	for (TFieldIterator<FProperty> PropIt(Function); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
	{
		const FProperty* Param = *PropIt;
		if (!Param->HasAnyPropertyFlags(CPF_OutParm) || Param->HasAnyPropertyFlags(CPF_ReferenceParm))
		{
			FEdGraphPinType PinType;
			bResult &= Schema->ConvertPropertyToPinType(Param, /*out*/ PinType);
			bResult &= (nullptr != CENode->CreateUserDefinedPin(Param->GetFName(), PinType, EGPD_Output));
		}
	}
	return bResult;
}

bool FAsyncTaskHelper::HandleDelegateImplementation(
	const FMulticastDelegateProperty* CurrentProperty,
	UEdGraphPin* ProxyObjectPin, UEdGraphPin*& InOutLastThenPin, UEdGraphPin*& OutLastActivatedThenPin,
	UK2Node* CurrentNode, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext)
{
	bool bIsErrorFree = true;
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();
	check(CurrentProperty && ProxyObjectPin && InOutLastThenPin && CurrentNode && SourceGraph && Schema);

	UEdGraphPin* PinForCurrentDelegateProperty = CurrentNode->FindPin(CurrentProperty->GetFName());
	if (!PinForCurrentDelegateProperty || (UEdGraphSchema_K2::PC_Exec != PinForCurrentDelegateProperty->PinType.PinCategory))
	{
		const FText ErrorMessage = FText::Format(LOCTEXT("WrongDelegateProperty", "BaseAsyncTask: Cannot find execution pin for delegate "), FText::FromString(CurrentProperty->GetName()));
		CompilerContext.MessageLog.Error(*ErrorMessage.ToString(), CurrentNode);
		return false;
	}

	UK2Node_CustomEvent* CurrentCENode = CompilerContext.SpawnIntermediateNode<UK2Node_CustomEvent>(CurrentNode, SourceGraph);
	{
		UK2Node_AddDelegate* AddDelegateNode = CompilerContext.SpawnIntermediateNode<UK2Node_AddDelegate>(CurrentNode, SourceGraph);
		AddDelegateNode->SetFromProperty(CurrentProperty, false, CurrentProperty->GetOwnerClass());
		AddDelegateNode->AllocateDefaultPins();
		bIsErrorFree &= Schema->TryCreateConnection(AddDelegateNode->FindPinChecked(UEdGraphSchema_K2::PN_Self), ProxyObjectPin);
		bIsErrorFree &= Schema->TryCreateConnection(InOutLastThenPin, AddDelegateNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute));
		InOutLastThenPin = AddDelegateNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);
		CurrentCENode->CustomFunctionName = *FString::Printf(TEXT("%s_%s"), *CurrentProperty->GetName(), *CompilerContext.GetGuid(CurrentNode));
		CurrentCENode->AllocateDefaultPins();

		bIsErrorFree &= CreateDelegateForNewFunction(AddDelegateNode->GetDelegatePin(), CurrentCENode->GetFunctionName(), CurrentNode, SourceGraph, CompilerContext);
		bIsErrorFree &= CopyEventSignature(CurrentCENode, AddDelegateNode->GetDelegateSignature(), Schema);
	}

	OutLastActivatedThenPin = CurrentCENode->FindPinChecked(UEdGraphSchema_K2::PN_Then);

	// CREATE CHAIN OF ASSIGMENTS
	// GATHER OUTPUT PARAMETERS AND PAIR THEM WITH LOCAL VARIABLES
	TArray<FOutputPinAndLocalVariable> VariableOutputs;
	{
		UEdGraphPin* CurrentPin = CurrentNode->FindPin(CurrentProperty->GetFName());
		const FEdGraphPinType& PinType = CurrentPin->PinType;
		UK2Node_TemporaryVariable* TempVarOutput = CompilerContext.SpawnInternalVariable(
			CurrentNode, PinType.PinCategory, PinType.PinSubCategory, PinType.PinSubCategoryObject.Get(), PinType.ContainerType, PinType.PinValueType);
		bIsErrorFree &= TempVarOutput->GetVariablePin() && CompilerContext.MovePinLinksToIntermediate(*CurrentPin, *TempVarOutput->GetVariablePin()).CanSafeConnect();
		VariableOutputs.Add(FAsyncTaskHelper::FOutputPinAndLocalVariable(CurrentPin, TempVarOutput));
	}
	for (const FOutputPinAndLocalVariable& OutputPair : VariableOutputs)
	{
		UEdGraphPin* PinWithData = CurrentCENode->FindPin(OutputPair.OutputPin->PinName);
		if (PinWithData == nullptr)
		{
			/*FText ErrorMessage = FText::Format(LOCTEXT("MissingDataPin", "ICE: Pin @@ was expecting a data output pin named {0} on @@ (each delegate must have the same signature)"), FText::FromString(OutputPair.OutputPin->PinName));
			CompilerContext.MessageLog.Error(*ErrorMessage.ToString(), OutputPair.OutputPin, CurrentCENode);
			return false;*/
			continue;
		}
	
		UK2Node_AssignmentStatement* AssignNode = CompilerContext.SpawnIntermediateNode<UK2Node_AssignmentStatement>(CurrentNode, SourceGraph);
		AssignNode->AllocateDefaultPins();
		bIsErrorFree &= Schema->TryCreateConnection(OutLastActivatedThenPin, AssignNode->GetExecPin());
		bIsErrorFree &= Schema->TryCreateConnection(OutputPair.TempVar->GetVariablePin(), AssignNode->GetVariablePin());
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetVariablePin());
		bIsErrorFree &= Schema->TryCreateConnection(AssignNode->GetValuePin(), PinWithData);
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetValuePin());
	
		OutLastActivatedThenPin = AssignNode->GetThenPin();
	}

	bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*PinForCurrentDelegateProperty, *OutLastActivatedThenPin).CanSafeConnect();
	return bIsErrorFree;
}

#pragma endregion 

#undef LOCTEXT_NAMESPACE
