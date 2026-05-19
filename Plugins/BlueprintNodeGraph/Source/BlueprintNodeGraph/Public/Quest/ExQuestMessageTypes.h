// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ExQuestMessageTypes.generated.h"

/** Payload for Quest.Event.Objective.Progress (GameplayMessageRouter). */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestObjectiveProgressMessage
{
	GENERATED_BODY()

	/** Objective configuration tag (required). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag ObjectiveTag;

	/** Progress delta (must be > 0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "1"))
	int32 Delta = 1;

	/** Optional: skip TaskId lookup when set. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag TaskId;
};

/** Native tag names registered in Config/DefaultGameplayTags.ini */
namespace ExQuestMessageTags
{
	BLUEPRINTNODEGRAPH_API FGameplayTag GetObjectiveProgressChannel();
}
