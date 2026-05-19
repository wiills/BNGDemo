// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/K2Nodes/ExK2Node_AsyncBase.h"
#include "ExK2Node_WaitAll.generated.h"

/** 多路汇入：全部支路报告后继续（EExBranchMode::All）。 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_WaitAll : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_WaitAll(const FObjectInitializer& ObjectInitializer);

	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual FText GetMenuCategory() const override;
};
