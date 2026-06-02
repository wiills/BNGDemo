// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/Details/ExClassedAsyncNodeDetails.h"

#include "BlueprintTool/K2Nodes/ExK2Node_ClassedAsyncBase.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "EdGraphSchema_K2.h"
#include "EdGraph/EdGraphPin.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "GameplayTagContainer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/STextComboBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "FExClassedAsyncNodeDetails"

TSharedRef<IDetailCustomization> FExClassedAsyncNodeDetails::MakeInstance()
{
	return MakeShareable(new FExClassedAsyncNodeDetails());
}

void FExClassedAsyncNodeDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	DetailBuilderPtr = &DetailBuilder;
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if (Objects.Num() != 1)
	{
		return;
	}

	Node = Cast<UExK2Node_ClassedAsyncBase>(Objects[0]);
	if (!Node.IsValid())
	{
		return;
	}

	DetailBuilder.EditCategory(
		TEXT("NodeInfo"),
		LOCTEXT("NodeInfoCategory", "Node Info"),
		ECategoryPriority::Important);

	IDetailCategoryBuilder& TaskConfigCat = DetailBuilder.EditCategory(
		TEXT("TaskConfig"),
		LOCTEXT("TaskConfigCategory", "Task Config"),
		ECategoryPriority::Default);

	BuildClassRow(TaskConfigCat);
	BuildParamRows(TaskConfigCat);
}

void FExClassedAsyncNodeDetails::BuildClassRow(IDetailCategoryBuilder& Category)
{
	UEdGraphPin* ClassPin = Node->GetClassPin();
	if (!ClassPin)
	{
		return;
	}

	UClass* CurrentClass = Node->GetClassToSpawn();
	FText CurrentClassText = CurrentClass ? FText::FromString(CurrentClass->GetName()) : FText::GetEmpty();

	Category.AddCustomRow(LOCTEXT("TaskClassRowLabel", "TaskClass"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TaskClassRowLabel", "TaskClass"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		.ValueContent()
		.MinDesiredWidth(250.f)
		[
			SNew(SClassPropertyEntryBox)
			.MetaClass(Node->GetValidBaseClass())
			.AllowAbstract(false)
			.AllowNone(true)
			.SelectedClass_Lambda([this]()
			{
				return Node.IsValid() ? Node->GetClassToSpawn() : nullptr;
			})
			.OnSetClass_Lambda([this](const UClass* NewClass)
			{
				OnClassChanged(NewClass);
			})
		];
}

void FExClassedAsyncNodeDetails::BuildParamRows(IDetailCategoryBuilder& Category)
{
	UClass* CurrentClass = Node->GetClassToSpawn();
	if (!CurrentClass)
	{
		return;
	}

	TArray<const FProperty*> Properties;
	Node->GetExposedPropertiesForClass(CurrentClass, Properties);

	for (const FProperty* Property : Properties)
	{
		FName PinName = Property->GetFName();
		UEdGraphPin* Pin = Node->FindPin(PinName);
		if (!Pin)
		{
			continue;
		}

		FText DisplayName = Property->GetDisplayNameText();
		FText Tooltip = Property->GetToolTipText();

		Category.AddCustomRow(DisplayName)
			.NameContent()
			[
				SNew(STextBlock)
				.Text(DisplayName)
				.ToolTipText(Tooltip)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.ValueContent()
			.MinDesiredWidth(250.f)
			[
				CreatePropertyEditor(Property, Pin)
			];
	}
}

TSharedRef<SWidget> FExClassedAsyncNodeDetails::CreatePropertyEditor(const FProperty* Property, UEdGraphPin* Pin)
{
	// 1. GameplayTag
	if (const FStructProperty* StructProp = CastField<FStructProperty>(Property))
	{
		if (StructProp->Struct == FGameplayTag::StaticStruct())
		{
			FGameplayTag CurrentTag;
			FString StringValue = Pin->GetDefaultAsString();
			if (!StringValue.IsEmpty())
			{
				CurrentTag = FGameplayTag::RequestGameplayTag(FName(*StringValue), false);
				if (!CurrentTag.IsValid())
				{
					FString TagNameStr;
					if (FParse::Value(*StringValue, TEXT("TagName="), TagNameStr))
					{
						TagNameStr = TagNameStr.TrimQuotes();
						CurrentTag = FGameplayTag::RequestGameplayTag(FName(*TagNameStr), false);
					}
				}
			}

			return SNew(SEditableTextBox)
				.Text(FText::FromString(CurrentTag.ToString()))
				.ToolTipText(FText::FromString(Property->GetMetaData(TEXT("Categories"))))
				.OnTextCommitted(this, &FExClassedAsyncNodeDetails::OnStringChanged, Pin->PinName);
		}
	}

	// 2. Bool
	if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
	{
		bool bValue = Pin->GetDefaultAsString() == TEXT("true");
		return SNew(SCheckBox)
			.IsChecked(bValue ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.OnCheckStateChanged_Lambda([this, PinName = Pin->PinName](ECheckBoxState NewState)
			{
				OnBoolChanged(PinName, NewState);
			});
	}

	// 3. Enum
	UEnum* Enum = nullptr;
	if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
	{
		Enum = EnumProp->GetEnum();
	}
	else if (const FByteProperty* ByteProp = CastField<FByteProperty>(Property))
	{
		Enum = ByteProp->Enum;
	}

	if (Enum)
	{
		int32 CurrentIndex = FCString::Atoi(*Pin->GetDefaultAsString());
		TArray<TSharedPtr<FString>> Options;
		for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
		{
			Options.Add(MakeShared<FString>(Enum->GetDisplayNameTextByIndex(i).ToString()));
		}

		return SNew(STextComboBox)
			.OptionsSource(&Options)
			.InitiallySelectedItem(Options.IsValidIndex(CurrentIndex) ? Options[CurrentIndex] : Options[0])
			.OnSelectionChanged_Lambda([this, PinName = Pin->PinName, Enum](TSharedPtr<FString> NewSelection, ESelectInfo::Type)
			{
				if (NewSelection.IsValid())
				{
					for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
					{
						if (Enum->GetDisplayNameTextByIndex(i).ToString() == *NewSelection)
						{
							OnEnumChanged(PinName, i, Enum);
							return;
						}
					}
				}
			});
	}

	// 4. Numeric (int/float/double)
	if (const FNumericProperty* NumProp = CastField<FNumericProperty>(Property))
	{
		FString CurrentValue = Pin->GetDefaultAsString();
		if (NumProp->IsFloatingPoint())
		{
			float CurrentFloat = FCString::Atof(*CurrentValue);
			return SNew(SNumericEntryBox<float>)
				.Value(CurrentFloat)
				.OnValueCommitted_Lambda([this, PinName = Pin->PinName](float NewValue, ETextCommit::Type)
				{
					OnStringChanged(FText::FromString(FString::SanitizeFloat(NewValue)), ETextCommit::Type::OnEnter, PinName);
				});
		}
		else
		{
			int32 CurrentInt = FCString::Atoi(*CurrentValue);
			return SNew(SNumericEntryBox<int32>)
				.Value(CurrentInt)
				.OnValueCommitted_Lambda([this, PinName = Pin->PinName](int32 NewValue, ETextCommit::Type)
				{
					OnStringChanged(FText::FromString(FString::FromInt(NewValue)), ETextCommit::Type::OnEnter, PinName);
				});
		}
	}

	// 5. Default: string editor
	FString CurrentValue = Pin->GetDefaultAsString();
	return SNew(SEditableTextBox)
		.Text(FText::FromString(CurrentValue))
		.OnTextCommitted(this, &FExClassedAsyncNodeDetails::OnStringChanged, Pin->PinName);
}

void FExClassedAsyncNodeDetails::RefreshDetailPanel()
{
	if (DetailBuilderPtr)
	{
		DetailBuilderPtr->ForceRefreshDetails();
	}
}

void FExClassedAsyncNodeDetails::OnClassChanged(const UClass* NewClass)
{
	if (!Node.IsValid())
	{
		return;
	}

	if (UEdGraphPin* ClassPin = Node->GetClassPin())
	{
		ClassPin->DefaultObject = const_cast<UClass*>(NewClass);
		Node->PinDefaultValueChanged(ClassPin);
		RefreshDetailPanel();
	}
}

void FExClassedAsyncNodeDetails::OnGameplayTagChanged(FName PinName, FGameplayTag NewTag)
{
	if (!Node.IsValid())
	{
		return;
	}

	if (UEdGraphPin* Pin = Node->FindPin(PinName))
	{
		Pin->DefaultValue = NewTag.ToString();
		FBlueprintEditorUtils::MarkBlueprintAsModified(Node->GetBlueprint());
	}
}

void FExClassedAsyncNodeDetails::OnBoolChanged(FName PinName, ECheckBoxState NewState)
{
	if (!Node.IsValid())
	{
		return;
	}

	if (UEdGraphPin* Pin = Node->FindPin(PinName))
	{
		Pin->DefaultValue = (NewState == ECheckBoxState::Checked) ? TEXT("true") : TEXT("false");
		FBlueprintEditorUtils::MarkBlueprintAsModified(Node->GetBlueprint());
	}
}

void FExClassedAsyncNodeDetails::OnStringChanged(const FText& NewText, ETextCommit::Type CommitType, FName PinName)
{
	if (!Node.IsValid())
	{
		return;
	}

	if (UEdGraphPin* Pin = Node->FindPin(PinName))
	{
		Pin->DefaultValue = NewText.ToString();
		FBlueprintEditorUtils::MarkBlueprintAsModified(Node->GetBlueprint());
	}
}

void FExClassedAsyncNodeDetails::OnEnumChanged(FName PinName, int32 NewIndex, UEnum* Enum)
{
	if (!Node.IsValid())
	{
		return;
	}

	if (UEdGraphPin* Pin = Node->FindPin(PinName))
	{
		Pin->DefaultValue = FString::FromInt(NewIndex);
		FBlueprintEditorUtils::MarkBlueprintAsModified(Node->GetBlueprint());
	}
}

#undef LOCTEXT_NAMESPACE
