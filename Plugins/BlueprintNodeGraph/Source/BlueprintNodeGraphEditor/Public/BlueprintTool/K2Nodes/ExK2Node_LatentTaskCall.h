// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/K2Nodes/ExK2Node_ClassedAsyncBase.h"
#include "UObject/ObjectMacros.h"
#include "Templates/SubclassOf.h"
#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"
#include "ExK2Node_LatentTaskCall.generated.h"

class FBlueprintActionDatabaseRegistrar;
class UEdGraph;
class UEdGraphPin;
class UEdGraphSchema;
class UEdGraphSchema_K2;

UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_LatentTaskCall : public UExK2Node_ClassedAsyncBase
{
	GENERATED_BODY()
	
public:
	UExK2Node_LatentTaskCall(const FObjectInitializer& ObjectInitializer);

	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	bool ValidateClassPin(class FKismetCompilerContext& CompilerContext, bool bGenerateErrors);
	
	UEdGraphPin* GetResultPin() const;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	static void RegisterSpecializedTaskNodeClass(TSubclassOf<UExK2Node_LatentTaskCall> NodeClass);
	
protected:
	static bool HasDedicatedNodeClass(TSubclassOf<UExBase_AsyncAction> TaskClass);
	virtual bool IsHandling(TSubclassOf<UExBase_AsyncAction> TaskClass) const { return true; }

private:
	static TArray<TWeakObjectPtr<UClass> > NodeClasses;
};
