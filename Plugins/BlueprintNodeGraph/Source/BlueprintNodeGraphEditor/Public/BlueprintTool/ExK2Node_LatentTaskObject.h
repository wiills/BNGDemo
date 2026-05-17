// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_AsyncBase.h"
#include "UObject/ObjectMacros.h"
#include "K2Node_BaseAsyncTask.h"
#include "BlueprintTool/ExAsyncActionBase.h"
#include "ExK2Node_LatentTaskObject.generated.h"

class FBlueprintActionDatabaseRegistrar;
class UEdGraph;
class UEdGraphPin;
class UEdGraphSchema;
class UEdGraphSchema_K2;

UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_LatentTaskObject : public UExK2Node_AsyncBase
{
	GENERATED_UCLASS_BODY()

	/************** UEdGraphNode interface **************/
	// Can this node be created under the specified schema
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const override;
	virtual void AllocateDefaultPins() override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual bool ShouldShowNodeProperties() const override {return true;}  // Show Node in DetailView Panel
	/************** UEdGraphNode interface **************/

	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	////////////////////// pins //////////////////////
	void CreatePinsForClass(UClass* InClass);
	UEdGraphPin* GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;
	UClass* GetClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;
	/** Get the output-param pin */
	UEdGraphPin* GetResultPin() const;
	/** Get the then output pin */
	UEdGraphPin* GetThenPin() const;
	////////////////////// pins //////////////////////

private:
	UPROPERTY()
	TArray<FName> SpawnParamPins;
};
