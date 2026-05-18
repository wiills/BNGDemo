// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExK2Node_AsyncBase.h"
#include "ExK2Node_WaitCount.generated.h"

/** 多路汇入：成功报告次数达到 Required Success Count 后继续（EExWaitBranchCompletionMode::Count）。 */
UCLASS()
class BLUEPRINTNODEGRAPHEDITOR_API UExK2Node_WaitCount : public UExK2Node_AsyncBase
{
	GENERATED_BODY()

public:
	UExK2Node_WaitCount(const FObjectInitializer& ObjectInitializer);

	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual FText GetMenuCategory() const override;
};
