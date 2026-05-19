// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExQuestAgentActor.generated.h"

class UExQuestDataAsset;
class USceneComponent;

/**
 * Per-level quest bootstrap: place in map (or spawn) with an instanced QuestDataAsset reference.
 * Authority loads into UExQuestManagerSubsystem and replicates via GameState UExQuestReplicationComponent.
 * Defaults: bReplicates, bAlwaysRelevant, DORM_Never — stays net-relevant (avoids distance/LOD culling breaking BeginPlay flow).
 * Does not replace GameState replication; only supplies which DA to load for this map.
 */
UCLASS(Blueprintable, BlueprintType)
class BLUEPRINTNODEGRAPH_API AExQuestAgentActor : public AActor
{
	GENERATED_BODY()

public:
	AExQuestAgentActor();

	/** Quest set for this map / sub-level (configure per instance in the level). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (ExposeOnSpawn = true))
	TObjectPtr<UExQuestDataAsset> QuestDataAsset = nullptr;

	/** When true, keep runtime progress if QuestSetId matches the loaded asset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bPreserveRuntime = false;

	/** Server / Standalone: load on BeginPlay when QuestDataAsset is set. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bAutoLoadOnBeginPlay = true;

	/** Load QuestDataAsset on authority (safe for Standalone / dedicated server). */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void LoadQuestFromConfiguredAsset();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TObjectPtr<USceneComponent> SceneRoot;
};
