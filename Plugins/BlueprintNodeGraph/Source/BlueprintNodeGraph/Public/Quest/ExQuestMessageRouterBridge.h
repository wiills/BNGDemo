// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Quest/ExQuestMessageTypes.h"
#include "ExQuestMessageRouterBridge.generated.h"

/**
 * Forwards GameplayMessageRouter events to UExQuestManagerSubsystem.
 * Listens on Quest.Event.Objective.Progress.
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExQuestMessageRouterBridge : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void RegisterListeners();
	void UnregisterListeners();

	void HandleObjectiveProgress(FGameplayTag Channel, const FExQuestObjectiveProgressMessage& Message);

	FGameplayMessageListenerHandle ObjectiveProgressListenerHandle;
};
