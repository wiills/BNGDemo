// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/SGraphNode_ExAsyncDebug.h"

#include "BlueprintTool/ExBlueprintNodeGraphDebugSubsystem.h"
#include "Rendering/DrawElements.h"
#include "Editor.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "K2Node.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SGraphNode_ExAsyncDebug"

static UObject* ResolvePlayOuterForBlueprint(UBlueprint* BP)
{
	if (!BP || !GEditor || !GEditor->PlayWorld)
	{
		return nullptr;
	}

	UWorld* World = GEditor->PlayWorld;
	UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(BP->GeneratedClass);
	if (!World || !BPGC)
	{
		return nullptr;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (It->IsA(BPGC))
		{
			return *It;
		}
	}

	return nullptr;
}

void SGraphNode_ExAsyncDebug::Construct(const FArguments& InArgs, UK2Node* InNode)
{
	GraphNode = InNode;
	SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();

	if (FSlateApplication::IsInitialized())
	{
		RegisterActiveTimer(
			0.05f,
			FWidgetActiveTimerDelegate::CreateSP(this, &SGraphNode_ExAsyncDebug::HandleBubbleRefresh));
	}
}

void SGraphNode_ExAsyncDebug::UpdateGraphNode()
{
	SGraphNodeK2Base::UpdateGraphNode();
}

bool SGraphNode_ExAsyncDebug::FetchBubbleText(FString& OutText)
{
	if (!GEditor || !GEditor->PlayWorld || !GraphNode)
	{
		return false;
	}

	UK2Node* K2Node = Cast<UK2Node>(GraphNode);
	if (!K2Node)
	{
		return false;
	}

	UBlueprint* BP = K2Node->GetBlueprint();
	if (!BP)
	{
		return false;
	}

	UGameInstance* GI = GEditor->PlayWorld->GetGameInstance();
	if (!GI)
	{
		return false;
	}

	UExBlueprintNodeGraphDebugSubsystem* Sub = GI->GetSubsystem<UExBlueprintNodeGraphDebugSubsystem>();
	if (!Sub)
	{
		return false;
	}

	UObject* PreferredOuter = ResolvePlayOuterForBlueprint(BP);
	return Sub->TryGetBubbleTextAuto(PreferredOuter, K2Node->NodeGuid, OutText);
}

EActiveTimerReturnType SGraphNode_ExAsyncDebug::HandleBubbleRefresh(double InCurrentTime, float InDeltaTime)
{
	FString NewText;
	const bool bHas = FetchBubbleText(NewText);
	if (bHas)
	{
		if (!NewText.Equals(CachedBubbleText))
		{
			CachedBubbleText = NewText;
			Invalidate(EInvalidateWidgetReason::Paint);
		}
	}
	else if (!CachedBubbleText.IsEmpty())
	{
		CachedBubbleText.Empty();
		Invalidate(EInvalidateWidgetReason::Paint);
	}
	return EActiveTimerReturnType::Continue;
}

int32 SGraphNode_ExAsyncDebug::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const int32 LastLayer = SGraphNodeK2Base::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId,
		InWidgetStyle, bParentEnabled);

	if (CachedBubbleText.IsEmpty())
	{
		return LastLayer;
	}

	{
		const FSlateFontInfo FontInfo = FAppStyle::GetFontStyle(TEXT("BoldFont"));
		const FVector2D Position(8.f, -20.f);

		const FPaintGeometry TextGeom = AllottedGeometry.ToPaintGeometry(
			FVector2f(FMath::Max(180.f, CachedBubbleText.Len() * 7.f), 24.f),
			FSlateLayoutTransform(Position));

		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LastLayer + 1,
			TextGeom,
			FAppStyle::GetBrush(TEXT("WhiteBrush")),
			ESlateDrawEffect::None,
			FLinearColor(0.55f, 0.38f, 0.28f, 0.92f));

		FSlateDrawElement::MakeText(
			OutDrawElements,
			LastLayer + 2,
			TextGeom,
			CachedBubbleText,
			FontInfo,
			ESlateDrawEffect::None,
			FLinearColor::White);
	}

	return LastLayer + 2;
}

#undef LOCTEXT_NAMESPACE
