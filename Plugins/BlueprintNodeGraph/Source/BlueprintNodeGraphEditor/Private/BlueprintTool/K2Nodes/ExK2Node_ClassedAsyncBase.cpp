// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/K2Nodes/ExK2Node_ClassedAsyncBase.h"

#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"

#define LOCTEXT_NAMESPACE "UExK2Node_ClassedAsyncBase"

namespace ExK2NodeClassedAsyncBasePrivate
{
void GetIgnorePropertyList(const UClass* ProxyFactoryClass, FName ProxyFactoryFunctionName, TArray<FString>& OutIgnorePropertyList)
{
	const UFunction* ProxyFunction = ProxyFactoryClass ? ProxyFactoryClass->FindFunctionByName(ProxyFactoryFunctionName) : nullptr;
	if (!ProxyFunction)
	{
		return;
	}

	const FString& IgnorePropertyListStr = ProxyFunction->GetMetaData(FName(TEXT("HideSpawnParms")));
	if (!IgnorePropertyListStr.IsEmpty())
	{
		IgnorePropertyListStr.ParseIntoArray(OutIgnorePropertyList, TEXT(","), true);
	}
}

bool IsExposedSpawnProperty(const FProperty* Property, const TArray<FString>& IgnorePropertyList)
{
	if (!Property)
	{
		return false;
	}

	const bool bIsDelegate = Property->IsA(FMulticastDelegateProperty::StaticClass());
	const bool bIsExposedToSpawn = UEdGraphSchema_K2::IsPropertyExposedOnSpawn(Property);
	const bool bIsSettableExternally = !Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance);

	return bIsExposedToSpawn &&
		!Property->HasAnyPropertyFlags(CPF_Parm) &&
		bIsSettableExternally &&
		Property->HasAllPropertyFlags(CPF_BlueprintVisible) &&
		!bIsDelegate &&
		!IgnorePropertyList.Contains(Property->GetName()) &&
		FBlueprintEditorUtils::PropertyStillExists(const_cast<FProperty*>(Property));
}
}

UExK2Node_ClassedAsyncBase::UExK2Node_ClassedAsyncBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UExK2Node_ClassedAsyncBase::AllocateDefaultPins()
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

	if (ShouldHideClassPin())
	{
		if (UEdGraphPin* ClassPin = GetClassPin())
		{
			ClassPin->bHidden = true;
		}
	}
}

void UExK2Node_ClassedAsyncBase::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();
	if (UClass* UseSpawnClass = GetClassToSpawn(&OldPins))
	{
		CreatePinsForClass(UseSpawnClass);
	}

	// 恢复旧引脚的默认值和连线
	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	for (UEdGraphPin* OldPin : OldPins)
	{
		UEdGraphPin* NewPin = FindPin(OldPin->PinName);
		if (NewPin && NewPin->Direction == OldPin->Direction)
		{
			if (NewPin->PinType == OldPin->PinType)
			{
				NewPin->DefaultValue = OldPin->DefaultValue;
				NewPin->DefaultObject = OldPin->DefaultObject;
				NewPin->DefaultTextValue = OldPin->DefaultTextValue;
			}
			K2Schema->MovePinLinks(*OldPin, *NewPin);
		}
	}

	RestoreSplitPins(OldPins);
}

void UExK2Node_ClassedAsyncBase::PostLoad()
{
	Super::PostLoad();
	SynchronizePinsForCurrentClass(false, false);
}

void UExK2Node_ClassedAsyncBase::PostReconstructNode()
{
	Super::PostReconstructNode();
	SynchronizePinsForCurrentClass(false, false);
}

UEdGraphPin* UExK2Node_ClassedAsyncBase::GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch /*= nullptr*/) const
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

UClass* UExK2Node_ClassedAsyncBase::GetClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch /*= nullptr*/) const
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

	if (UClass* ValidBase = GetValidBaseClass())
	{
		if (UseSpawnClass && !UseSpawnClass->IsChildOf(ValidBase))
		{
			UseSpawnClass = ValidBase;
		}
	}
	if (UseSpawnClass && !IsSpawnClassValid(UseSpawnClass))
	{
		UseSpawnClass = GetValidBaseClass();
	}

	return UseSpawnClass;
}

void UExK2Node_ClassedAsyncBase::CreatePinsForClass(UClass* InClass)
{
	check(InClass != nullptr);

	SpawnParamPins.Reset();
	SynchronizePinsForClass(InClass);
}

bool UExK2Node_ClassedAsyncBase::SynchronizePinsForClass(UClass* InClass)
{
	if (!InClass)
	{
		return false;
	}

	const UEdGraphSchema_K2* K2Schema = GetDefault<UEdGraphSchema_K2>();
	const UObject* const ClassDefaultObject = InClass->GetDefaultObject(false);

	bool bChanged = false;
	bool bHasAdvancedPins = false;
	TArray<FString> IgnorePropertyList;
	TSet<FName> ValidSpawnParamPins;
	ExK2NodeClassedAsyncBasePrivate::GetIgnorePropertyList(ProxyFactoryClass, ProxyFactoryFunctionName, IgnorePropertyList);

	for (TFieldIterator<FProperty> PropertyIt(InClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		const FProperty* Property = *PropertyIt;
		if (!ExK2NodeClassedAsyncBasePrivate::IsExposedSpawnProperty(Property, IgnorePropertyList))
		{
			continue;
		}

		UEdGraphPin* Pin = FindPin(Property->GetFName());
		if (!Pin)
		{
			Pin = CreatePin(EGPD_Input, NAME_None, Property->GetFName());
			check(Pin);
			const bool bPinGood = K2Schema->ConvertPropertyToPinType(Property, Pin->PinType);
			check(bPinGood);
			bChanged = true;

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

		ValidSpawnParamPins.Add(Pin->PinName);
		SpawnParamPins.AddUnique(Pin->PinName);

		const bool bAdvancedPin = Property->HasMetaData(TEXT("AdvancedDisplay")) || Property->HasAnyPropertyFlags(CPF_AdvancedDisplay);
		bHasAdvancedPins |= bAdvancedPin;
		if (Pin->bAdvancedView != bAdvancedPin)
		{
			Pin->bAdvancedView = bAdvancedPin;
			bChanged = true;
		}

		if (ShouldHideSpawnParamPins() && !Pin->bHidden)
		{
			Pin->bHidden = true;
			bChanged = true;
		}
	}

	for (int32 PinIndex = SpawnParamPins.Num() - 1; PinIndex >= 0; --PinIndex)
	{
		const FName PinName = SpawnParamPins[PinIndex];
		if (ValidSpawnParamPins.Contains(PinName))
		{
			continue;
		}

		if (UEdGraphPin* StalePin = FindPin(PinName))
		{
			StalePin->BreakAllPinLinks();
			Pins.Remove(StalePin);
			bChanged = true;
		}
		SpawnParamPins.RemoveAtSwap(PinIndex);
	}

	if (bHasAdvancedPins && AdvancedPinDisplay == ENodeAdvancedPins::NoPins)
	{
		AdvancedPinDisplay = ENodeAdvancedPins::Hidden;
		bChanged = true;
	}

	ApplyPinVisibility();
	return bChanged;
}

bool UExK2Node_ClassedAsyncBase::EnsureSpawnParamPinsUpToDate()
{
	return SynchronizePinsForCurrentClass(true, true);
}

bool UExK2Node_ClassedAsyncBase::SynchronizePinsForCurrentClass(bool bNotifyGraph, bool bMarkBlueprintModified)
{
	UClass* UseSpawnClass = GetClassToSpawn();
	if (!UseSpawnClass)
	{
		return false;
	}

	const bool bChanged = SynchronizePinsForClass(UseSpawnClass);
	if (!bChanged)
	{
		return false;
	}

	if (bNotifyGraph)
	{
		if (UEdGraph* Graph = GetGraph())
		{
			Graph->NotifyGraphChanged();
		}
	}

	if (bMarkBlueprintModified)
	{
		FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
	}

	return true;
}

void UExK2Node_ClassedAsyncBase::ApplyPinVisibility()
{
	if (!ShouldHideSpawnParamPins())
	{
		return;
	}

	for (const FName& PinName : SpawnParamPins)
	{
		if (UEdGraphPin* Pin = FindPin(PinName))
		{
			Pin->bHidden = true;
		}
	}
}

void UExK2Node_ClassedAsyncBase::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin->PinName == ExLatentTaskHelper::ClassPinName)
	{
		for (const FName& OldPinReference : SpawnParamPins)
		{
			if (UEdGraphPin* OldPin = FindPin(OldPinReference))
			{
				OldPin->BreakAllPinLinks();
				Pins.Remove(OldPin);
			}
		}

		SpawnParamPins.Reset();

		if (UClass* UseSpawnClass = GetClassToSpawn())
		{
			CreatePinsForClass(UseSpawnClass);
		}

		if (UEdGraph* Graph = GetGraph())
		{
			Graph->NotifyGraphChanged();
		}

		FBlueprintEditorUtils::MarkBlueprintAsModified(GetBlueprint());
	}
}

void UExK2Node_ClassedAsyncBase::GetExposedPropertiesForClass(UClass* InClass, TArray<const FProperty*>& OutProperties) const
{
	if (!InClass)
	{
		return;
	}

	TArray<FString> IgnorePropertyList;
	ExK2NodeClassedAsyncBasePrivate::GetIgnorePropertyList(ProxyFactoryClass, ProxyFactoryFunctionName, IgnorePropertyList);

	for (TFieldIterator<FProperty> PropertyIt(InClass, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		const FProperty* Property = *PropertyIt;
		if (ExK2NodeClassedAsyncBasePrivate::IsExposedSpawnProperty(Property, IgnorePropertyList))
		{
			OutProperties.Add(Property);
		}
	}
}

#undef LOCTEXT_NAMESPACE
