// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "GameplayTagContainer.h"

class UExK2Node_ClassedAsyncBase;
class UEdGraphPin;
class UEnum;

/**
 * Detail panel customization for classed async nodes (QuestTask, TimerTask, etc).
 * Shows Class dropdown + editable ExposeOnSpawn parameters in the Details panel.
 */
class FExClassedAsyncNodeDetails : public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	TWeakObjectPtr<UExK2Node_ClassedAsyncBase> Node;
	IDetailLayoutBuilder* DetailBuilderPtr = nullptr;

	void BuildClassRow(IDetailCategoryBuilder& Category);
	void BuildParamRows(IDetailCategoryBuilder& Category);

	void OnClassChanged(const UClass* NewClass);
	void RefreshDetailPanel();

	TSharedRef<SWidget> CreatePropertyEditor(const FProperty* Property, UEdGraphPin* Pin);

	// Value change handlers
	void OnGameplayTagChanged(FName PinName, FGameplayTag NewTag);
	void OnBoolChanged(FName PinName, ECheckBoxState NewState);
	void OnStringChanged(const FText& NewText, ETextCommit::Type CommitType, FName PinName);
	void OnEnumChanged(FName PinName, int32 NewIndex, UEnum* Enum);
};
