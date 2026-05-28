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
#include "BlueprintTool/LatentTasks/ExLatentTask_Custom.h"
#include "BlueprintTool/LatentTasks/ExLatentTask_Timer.h"

#define LOCTEXT_NAMESPACE "UExK2Node_TimerTask"

UExK2Node_TimerTask::UExK2Node_TimerTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProxyFactoryFunctionName = GET_FUNCTION_NAME_CHECKED(UExLatentTask_Custom, CreateProxy);
	ProxySetK2NodeInfoFunctionName = GET_FUNCTION_NAME_CHECKED(UExLatentTask_Custom, SetK2NodeInfo);
	ProxyActivateFunctionName = GET_FUNCTION_NAME_CHECKED(UExLatentTask_Custom, Activate);
	ProxyFactoryClass = UExLatentTask_Custom::StaticClass();
	ProxyClass = UExLatentTask_Timer::StaticClass();
}

FText UExK2Node_TimerTask::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
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

	for (const UStruct* TestStruct = ProxyClass; TestStruct; TestStruct = TestStruct->GetSuperStruct())
	{
		const bool bSafeHideThen = TestStruct->HasMetaData(TEXT("SafeHideThen"));
		if (bSafeHideThen && GetThenPin())
		{
			GetThenPin()->SafeSetHidden(true);
			break;
		}
	}

	if (UEdGraphPin* ClassPin = GetClassPin())
	{
		ClassPin->DefaultObject = UExLatentTask_Timer::StaticClass();
	}

	CreatePinsForClass(UExLatentTask_Timer::StaticClass());
}

FSlateIcon UExK2Node_TimerTask::GetIconAndTint(FLinearColor& OutColor) const
{
	static FSlateIcon Icon("EditorStyle", "GraphEditor.Default_16x");
	return Icon;
}

void UExK2Node_TimerTask::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();
	UClass* UseSpawnClass = GetClassToSpawn(&OldPins);
	if (UseSpawnClass != nullptr)
	{
		CreatePinsForClass(UseSpawnClass);
	}
	RestoreSplitPins(OldPins);
}

UEdGraphPin* UExK2Node_TimerTask::GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch) const
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

UClass* UExK2Node_TimerTask::GetClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch) const
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

	if (UseSpawnClass == nullptr)
	{
		UseSpawnClass = UExLatentTask_Timer::StaticClass();
	}
	else if (!UseSpawnClass->IsChildOf(UExLatentTask_Timer::StaticClass()))
	{
		UseSpawnClass = UExLatentTask_Timer::StaticClass();
	}

	return UseSpawnClass;
}

void UExK2Node_TimerTask::CreatePinsForClass(UClass* InClass)
{
	check(InClass != nullptr);

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	const UObject* const ClassDefaultObject = InClass->GetDefaultObject(false);

	SpawnParamPins.Reset();
	bool bHasAdvancedPins = false;

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
		const bool bIsDelegate = Property->IsA(FMulticastDelegateProperty::StaticClass());
		const bool bIsExposedToSpawn = UEdGraphSchema_K2::IsPropertyExposedOnSpawn(Property);
		const bool bIsSettableExternally = !Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance);

		if (bIsExposedToSpawn &&
			!Property->HasAnyPropertyFlags(CPF_Parm) &&
			bIsSettableExternally &&
			Property->HasAllPropertyFlags(CPF_BlueprintVisible) &&
			!bIsDelegate &&
			!IgnorePropertyList.Contains(Property->GetName()) &&
			(FindPin(Property->GetFName()) == nullptr))
		{
			UEdGraphPin* Pin = CreatePin(EGPD_Input, NAME_None, Property->GetFName());
			check(Pin);
			const bool bPinGood = K2Schema->ConvertPropertyToPinType(Property, Pin->PinType);
			check(bPinGood);
			SpawnParamPins.Add(Pin->PinName);

			const bool bAdvancedPin = Property->HasMetaData(TEXT("AdvancedDisplay"));
			Pin->bAdvancedView = bAdvancedPin;
			bHasAdvancedPins |= bAdvancedPin;

			if (ClassDefaultObject && K2Schema->PinDefaultValueIsEditable(*Pin))
			{
				FString DefaultValueAsString;
				const bool bDefaultValueSet = FBlueprintEditorUtils::PropertyValueToString(
					Property, reinterpret_cast<const uint8*>(ClassDefaultObject), DefaultValueAsString, this);
				check(bDefaultValueSet);
				K2Schema->SetPinAutogeneratedDefaultValue(Pin, DefaultValueAsString);
			}

			K2Schema->ConstructBasicPinTooltip(*Pin, Property->GetToolTipText(), Pin->PinToolTip);
		}
	}

	if (bHasAdvancedPins && AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
	{
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
	}
}

void UExK2Node_TimerTask::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin->PinName == ExLatentTaskHelper::ClassPinName)
	{
		TArray<UEdGraphPin*> RemovedPins;
		for (const FName& OldPinReference : SpawnParamPins)
		{
			if (UEdGraphPin* OldPin = FindPin(OldPinReference))
			{
				if (OldPin->HasAnyConnections())
				{
					RemovedPins.Add(OldPin);
				}
				Pins.Remove(OldPin);
			}
		}

		SpawnParamPins.Reset();

		if (UClass* UseSpawnClass = GetClassToSpawn())
		{
			CreatePinsForClass(UseSpawnClass);
		}

		RewireOldPinsToNewPins(RemovedPins, Pins, nullptr);

		if (UEdGraph* Graph = GetGraph())
		{
			Graph->NotifyGraphChanged();
		}

		FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
	}
}

UEdGraphPin* UExK2Node_TimerTask::GetResultPin() const
{
	UEdGraphPin* Pin = FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue);
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

void UExK2Node_TimerTask::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
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
