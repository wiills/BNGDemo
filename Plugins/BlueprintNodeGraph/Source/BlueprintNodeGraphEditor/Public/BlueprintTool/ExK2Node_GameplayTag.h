// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_AsyncBase.h"
#include "ExK2Node_GameplayTag.generated.h"

UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_GameplayTagListener : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_GameplayTagListener(const FObjectInitializer& ObjectInitializer);

	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
};

UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_GameplayTagQuery : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_GameplayTagQuery(const FObjectInitializer& ObjectInitializer);

	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
};

UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_GameplayTagModifier : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_GameplayTagModifier(const FObjectInitializer& ObjectInitializer);

	virtual void AllocateDefaultPins() override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
};
