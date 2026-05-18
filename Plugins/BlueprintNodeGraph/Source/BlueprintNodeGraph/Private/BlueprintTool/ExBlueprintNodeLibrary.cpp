// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExBlueprintNodeLibrary.h"

#include "DelayAction.h"
#include "GameplayTagContainer.h"
#include "BlueprintTool/ExWaitAction.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UExBlueprintNodeLibrary::WaitAction(const UObject* WorldContextObject, float Duration, FLatentActionInfo LatentInfo)
{
	if (UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr)
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		const auto OldAction = LatentActionManager.FindExistingAction<FExWaitAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);
		if (OldAction == nullptr)
		{
			const auto NewAction = new FExWaitAction(Duration, LatentInfo);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
			NewAction->HandleInput();
		}
		else
		{
			OldAction->HandleInput();
		}
	}
}

void UExBlueprintNodeLibrary::OnDelay(const UObject* WorldContext, float Duration, FLatentActionInfo LatentInfo)
{
	if (UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull) : nullptr)
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FDelayAction(Duration, LatentInfo));
	}
}



bool UExBlueprintNodeLibrary::HasGameplayTag(UObject* WorldContextObject, FGameplayTag TagToCheck)
{
	return TagToCheck.IsValid();
}

bool UExBlueprintNodeLibrary::HasAllTags(UObject* WorldContextObject, FGameplayTagContainer TagsToCheck)
{
	for (const FGameplayTag& Tag : TagsToCheck)
	{
		if (!Tag.IsValid())
		{
			return false;
		}
	}
	return true;
}

bool UExBlueprintNodeLibrary::HasAnyTag(UObject* WorldContextObject, FGameplayTagContainer TagsToCheck)
{
	for (const FGameplayTag& Tag : TagsToCheck)
	{
		if (Tag.IsValid())
		{
			return true;
		}
	}
	return false;
}