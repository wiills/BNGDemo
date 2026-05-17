// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_AsyncBase.h"
#include "ExK2Node_WaitCondition.generated.h"

/**
 * Wait Multi-Async-Input, Wait Condition Satisfied, Then Execute
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_WaitCondition : public UExK2Node_AsyncBase
{
	GENERATED_BODY()
	
public:
	UExK2Node_WaitCondition(const FObjectInitializer& ObjectInitializer);
	
	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual void AllocateDefaultPins() override;
	//~ End UEdGraphNode Interface
};
