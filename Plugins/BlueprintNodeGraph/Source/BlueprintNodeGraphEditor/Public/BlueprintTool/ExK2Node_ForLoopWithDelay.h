// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_AsyncBase.h"
#include "ExK2Node_ForLoopWithDelay.generated.h"

/**
 * For Loop, with delay interval
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_ForLoopWithDelay : public UExK2Node_AsyncBase
{
	GENERATED_BODY()
	
public:
	UExK2Node_ForLoopWithDelay(const FObjectInitializer& ObjectInitializer);
	
	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode Interface

	// virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
};
