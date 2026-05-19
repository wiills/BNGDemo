// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Quest/ExQuestTypes.h"
#include "Quest/ExQuestMessageTypes.h"
#include "ExQuestReplicationComponent.generated.h"

class UExQuestDataAsset;
class UExQuestManagerSubsystem;

/**
 * Replicates shared team quest progress (GameState component).
 * Add to your GameState (or use AExQuestGameStateBase). Server is authoritative.
 */
UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class BLUEPRINTNODEGRAPH_API UExQuestReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExQuestReplicationComponent();

	static UExQuestReplicationComponent* Get(const UObject* WorldContextObject);

	/** Standalone, listen-server host, or dedicated server. */
	UFUNCTION(BlueprintPure, Category = "Quest|Network")
	bool IsAuthorityEndpoint() const;

	UFUNCTION(BlueprintPure, Category = "Quest|Network")
	FExQuestRuntimeState GetReplicatedRuntimeState() const { return ReplicatedRuntimeState; }

	// --- Routed API (Standalone / Server local, Client -> Server RPC) ---

	static bool RouteUnlockQuest(UObject* WorldContextObject, const FGameplayTag& TaskId);
	static bool RouteIncrementQuestObjective(UObject* WorldContextObject, const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag, int32 Delta);
	static bool RouteCompleteQuestObjective(UObject* WorldContextObject, const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag);
	static bool RouteNotifyObjectiveProgressByTag(UObject* WorldContextObject, const FGameplayTag& ObjectiveTag, int32 Delta);
	static void RouteApplyRuntimeState(UObject* WorldContextObject, const FExQuestRuntimeState& RuntimeState);
	static void RouteLoadQuestFromAsset(UObject* WorldContextObject, UExQuestDataAsset* QuestAsset, bool bPreserveRuntime);
	static bool RouteLoadQuestProgressFromJson(UObject* WorldContextObject, const FString& JsonSaveData);
	static void RouteApplyObjectiveProgressMessage(UObject* WorldContextObject, const FExQuestObjectiveProgressMessage& Message);

	void PublishStateFromAuthorityManager(UExQuestManagerSubsystem* QuestManager);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ReplicatedRuntimeState();

	UFUNCTION()
	void OnRep_ReplicatedQuestAsset();

	void ApplyReplicationToLocalManagers();

	UFUNCTION(Server, Reliable)
	void Server_UnlockQuest(const FGameplayTag& TaskId);

	UFUNCTION(Server, Reliable)
	void Server_IncrementQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag, int32 Delta);

	UFUNCTION(Server, Reliable)
	void Server_CompleteQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag);

	UFUNCTION(Server, Reliable)
	void Server_NotifyObjectiveProgressByTag(const FGameplayTag& ObjectiveTag, int32 Delta);

	UFUNCTION(Server, Reliable)
	void Server_ApplyRuntimeState(const FExQuestRuntimeState& RuntimeState);

	UFUNCTION(Server, Reliable)
	void Server_LoadQuestFromAsset(UExQuestDataAsset* QuestAsset, bool bPreserveRuntime);

	UFUNCTION(Server, Reliable)
	void Server_LoadQuestProgressFromJson(const FString& JsonSaveData);

	UFUNCTION(Server, Reliable)
	void Server_ApplyObjectiveProgressMessage(const FExQuestObjectiveProgressMessage& Message);

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedRuntimeState)
	FExQuestRuntimeState ReplicatedRuntimeState;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedQuestAsset)
	TObjectPtr<UExQuestDataAsset> ReplicatedQuestAsset = nullptr;
};
