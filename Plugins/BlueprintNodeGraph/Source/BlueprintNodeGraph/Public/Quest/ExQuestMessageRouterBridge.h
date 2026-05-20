// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Quest/ExQuestMessageTypes.h"
#include "ExQuestMessageRouterBridge.generated.h"

/**
 * Forwards GameplayMessageRouter events to the quest manager when WITH_QUEST_MESSAGE_ROUTER=1.
 * UCLASS must stay outside preprocessor blocks (UHT requirement).
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExQuestMessageRouterBridge : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void HandleObjectiveProgress(FGameplayTag Channel, const FExQuestObjectiveProgressMessage& Message);
};
