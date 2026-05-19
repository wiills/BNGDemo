// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/K2Nodes/ExK2Node_AsyncBase.h"
#include "ExK2Node_WaitAny.generated.h"

/** Multi-branch merge: continue after any branch succeeds (EExBranchMode::Any). */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_WaitAny : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_WaitAny(const FObjectInitializer& ObjectInitializer);

	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual FText GetMenuCategory() const override;
};
