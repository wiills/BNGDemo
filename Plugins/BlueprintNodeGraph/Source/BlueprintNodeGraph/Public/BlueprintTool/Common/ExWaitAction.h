// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LatentActions.h"
#include "UObject/WeakObjectPtr.h"
#include "Engine/LatentActionManager.h"


/**
 * A simple delay action;
 */
class FExWaitAction : public FPendingLatentAction
{
public:
	int32 CurrentTimeLeft;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	FExWaitAction(int32 InDuration, const FLatentActionInfo& LatentInfo)
		: CurrentTimeLeft(InDuration)
		, ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		CurrentTimeLeft -= Response.ElapsedTime();
		Response.FinishAndTriggerIf(CurrentTimeLeft <= 0, ExecutionFunction, OutputLink, CallbackTarget);
	}

	virtual void HandleInput(){}

#if WITH_EDITOR
	// Returns a human readable description of the latent operation's current state
	virtual FString GetDescription() const override
	{
		static const FNumberFormattingOptions DelayTimeFormatOptions = FNumberFormattingOptions()
			.SetMinimumFractionalDigits(3)
			.SetMaximumFractionalDigits(3);
		return FText::Format(NSLOCTEXT("ExWaitAction", "ExWaitActionFmt", "CurrentTimeLeft ({0})"), FText::AsNumber(CurrentTimeLeft)).ToString();
	}
#endif
};
