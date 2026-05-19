// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/K2Nodes/ExK2Node_AsyncBase.h"
#include "ExK2Node_LoopDelay.generated.h"

/**
 * Wait Then Execute
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_LoopDelay : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_LoopDelay(const FObjectInitializer& ObjectInitializer);
	
	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode Interface
};
