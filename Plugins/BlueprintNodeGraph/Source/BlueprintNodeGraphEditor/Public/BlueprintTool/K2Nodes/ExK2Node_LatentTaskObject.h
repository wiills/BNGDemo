// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/K2Nodes/ExK2Node_ClassedAsyncBase.h"
#include "ExK2Node_LatentTaskObject.generated.h"

class FBlueprintActionDatabaseRegistrar;
class UEdGraph;
class UEdGraphPin;
class UEdGraphSchema;
class UEdGraphSchema_K2;

UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_LatentTaskObject : public UExK2Node_ClassedAsyncBase
{
	GENERATED_UCLASS_BODY()

	virtual bool CanCreateUnderSpecifiedSchema(const UEdGraphSchema* DesiredSchema) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;

	UEdGraphPin* GetResultPin() const;

	virtual UClass* GetValidBaseClass() const override;
	virtual bool IsSpawnClassValid(UClass* InClass) const override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
};
