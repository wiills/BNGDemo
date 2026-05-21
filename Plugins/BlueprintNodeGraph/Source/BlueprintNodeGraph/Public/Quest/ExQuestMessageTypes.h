// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ExQuestMessageTypes.generated.h"

/** Quest event kind carried by FExQuestMessagePayload (GameplayMessageRouter). */
UENUM(BlueprintType)
enum class EExQuestMessageType : uint8
{
	/** Increment objective progress on channel Quest.Event.Objective.Progress. */
	ObjectiveProgress UMETA(DisplayName = "Objective Progress"),
};

/** Unified payload for Quest GameplayMessageRouter broadcasts and listeners. */
USTRUCT(BlueprintType, meta = (DisplayName = "Quest Message Payload"))
struct BLUEPRINTNODEGRAPH_API FExQuestMessagePayload
{
	GENERATED_BODY()

	/** Which quest event this payload represents. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EExQuestMessageType MessageType = EExQuestMessageType::ObjectiveProgress;

	/** Objective configuration tag (ObjectiveProgress). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag ObjectiveTag;

	/** Progress delta (ObjectiveProgress, must be > 0). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ClampMin = "1"))
	int32 Delta = 1;

	/** Optional: skip TaskId lookup when set (ObjectiveProgress). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag TaskId;

	bool IsValid() const
	{
		switch (MessageType)
		{
		case EExQuestMessageType::ObjectiveProgress:
			return ObjectiveTag.IsValid() && Delta > 0;
		default:
			return false;
		}
	}
};

/** Native tag names registered in Config/DefaultGameplayTags.ini */
namespace ExQuestMessageTags
{
	BLUEPRINTNODEGRAPH_API FGameplayTag GetObjectiveProgressChannel();
	BLUEPRINTNODEGRAPH_API FGameplayTag GetChannelForMessageType(EExQuestMessageType MessageType);
}
