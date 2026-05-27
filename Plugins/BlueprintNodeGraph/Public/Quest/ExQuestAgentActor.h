// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExQuestAgentActor.generated.h"

class UExQuestDataAsset;
class USceneComponent;
class APlayerController;

/**
 * Per-level quest bootstrap: place in map (or spawn) with an instanced QuestDataAsset reference.
 * Authority loads into UExQuestManagerSubsystem and replicates via GameState UExQuestReplicationComponent.
 * Blueprint entry: Event On Server Ready (authority) / Event On Client Ready (local PlayerController).
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

	/** Manually retry client PlayerController detection (e.g. after custom spawn flow). */
	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (AdvancedDisplay = "true"))
	void NotifyClientReadyWhenAvailable();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Authority / Standalone: quest task chain, unlock flow (after auto Load when enabled). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest", meta = (DisplayName = "Event On Server Ready"))
	void EventOnServerReady();

	/** Local client / listen-server host: Create Widget, HUD (PlayerController is valid). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Quest", meta = (DisplayName = "Event On Client Ready"))
	void EventOnClientReady(APlayerController* PlayerController);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", AdvancedDisplay)
	bool bAutoFireServerReadyEvent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", AdvancedDisplay)
	bool bAutoFireClientReadyEvent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", AdvancedDisplay, meta = (ClampMin = "0.01", EditCondition = "bAutoFireClientReadyEvent"))
	float ClientReadyPollInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", AdvancedDisplay, meta = (ClampMin = "0.0", EditCondition = "bAutoFireClientReadyEvent"))
	float ClientReadyMaxWaitSeconds = 15.0f;

	void StartClientReadyPolling();
	void StopClientReadyPolling();
	void TryFireClientReadyEvent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> NotifiedClientPlayerController;

	FTimerHandle ClientReadyTimerHandle;
	float ClientReadyElapsedSeconds = 0.0f;
};
