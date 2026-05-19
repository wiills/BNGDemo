// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ExQuestGameStateBase.generated.h"

/**
 * Optional GameState with quest replication component pre-attached.
 * Or add UExQuestReplicationComponent to your own GameState Blueprint.
 */
UCLASS(Blueprintable)
class BLUEPRINTNODEGRAPH_API AExQuestGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	AExQuestGameStateBase();

	UFUNCTION(BlueprintPure, Category = "Quest|Network")
	class UExQuestReplicationComponent* GetQuestReplicationComponent() const { return QuestReplication; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Network")
	TObjectPtr<class UExQuestReplicationComponent> QuestReplication;
};
