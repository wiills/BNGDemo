// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Rendering/DrawElements.h"
#include "KismetNodes/SGraphNodeK2Base.h"

/**
 * ExK2 ???????PIE/SIE ????????? Delay ???????????? BlueprintNodeGraph ???????
 */
class BLUEPRINTNODEGRAPHEDITOR_API SGraphNode_ExAsyncDebug : public SGraphNodeK2Base
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_ExAsyncDebug) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UK2Node* InNode);

	virtual void UpdateGraphNode() override;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

private:
	EActiveTimerReturnType HandleBubbleRefresh(double InCurrentTime, float InDeltaTime);

	bool FetchBubbleText(FString& OutText);

	FString CachedBubbleText;
};
