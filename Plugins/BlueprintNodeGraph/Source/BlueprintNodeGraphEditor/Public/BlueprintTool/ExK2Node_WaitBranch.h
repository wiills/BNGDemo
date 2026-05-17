// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_AsyncBase.h"
#include "ExK2Node_WaitBranch.generated.h"

/**
 * Wait Multi-Async-Input Then Execute Once
 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_WaitBranch : public UExK2Node_AsyncBase
{
	GENERATED_BODY()
	
public:
	UExK2Node_WaitBranch(const FObjectInitializer& ObjectInitializer);
	
	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	//~ End UEdGraphNode Interface
};
