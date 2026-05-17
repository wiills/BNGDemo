// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_AsyncBase.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "BlueprintTool/ExAsyncActionBase.h"
#include "ExK2Node_LatentTaskCall.generated.h"

class FBlueprintActionDatabaseRegistrar;
class UEdGraph;
class UEdGraphPin;
class UEdGraphSchema;
class UEdGraphSchema_K2;

UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_LatentTaskCall : public UExK2Node_AsyncBase
{
	GENERATED_BODY()
	
public:
	UExK2Node_LatentTaskCall(const FObjectInitializer& ObjectInitializer);

	// Begin of UEdGraphNode interface
	virtual bool ShouldShowNodeProperties() const override { return true; }  // Show Node in DetailView Panel
	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	// End of UEdGraphNode interface

	bool ValidateClassPin(class FKismetCompilerContext& CompilerContext, bool bGenerateErrors);
	
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	void CreatePinsForClass(UClass* InClass);
	UEdGraphPin* GetClassPin(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;
	UClass* GetClassToSpawn(const TArray<UEdGraphPin*>* InPinsToSearch = nullptr) const;
	UEdGraphPin* GetResultPin() const;
	
	/** Get the then output pin */
	UEdGraphPin* GetThenPin() const;

	static void RegisterSpecializedTaskNodeClass(TSubclassOf<UExK2Node_LatentTaskCall> NodeClass);
	
protected:
	// dropdown menu, classes
	static bool HasDedicatedNodeClass(TSubclassOf<UExAsyncActionBase> TaskClass);

	virtual bool IsHandling(TSubclassOf<UExAsyncActionBase> TaskClass) const { return true; }

private:
	static TArray<TWeakObjectPtr<UClass> > NodeClasses;

	UPROPERTY()
	TArray<FName> SpawnParamPins;
};
