// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExQuestTypes.h"
#include "ExQuestManagerSubsystem.generated.h"

class UExQuestDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStateChanged, const FExQuestTask&, QuestTask);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestObjectiveUpdated, const FExQuestObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestProgressChanged, const FGameplayTag&, TaskId, float, CompletionPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestDataLoaded);

/** Game-instance quest manager */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExQuestManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStateChanged OnQuestStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestObjectiveUpdated OnQuestObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestProgressChanged OnQuestProgressChanged;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestDataLoaded OnQuestDataLoaded;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void LoadQuestData(const FExQuestData& QuestData);

	/** Load from DataAsset; optionally keep current runtime progress when QuestSetId matches */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void LoadQuestFromAsset(UExQuestDataAsset* QuestAsset, bool bPreserveRuntime = false);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	const FExQuestData& GetQuestData() const { return CurrentQuestData; }

	UFUNCTION(BlueprintCallable, Category = "Quest")
	FExQuestRuntimeState GetRuntimeState() const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ApplyRuntimeState(const FExQuestRuntimeState& RuntimeState);

	UFUNCTION(BlueprintPure, Category = "Quest")
	UExQuestDataAsset* GetLoadedQuestAsset() const { return LoadedQuestAsset; }

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool ActivateQuest(const FGameplayTag& TaskId);

	/** Locked -> Inactive when prerequisites are met */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool UnlockQuest(const FGameplayTag& TaskId);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool CompleteQuest(const FGameplayTag& TaskId);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool FailQuest(const FGameplayTag& TaskId);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool UpdateQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag, int32 NewProgress);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IncrementQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag, int32 Delta = 1);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool CompleteQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag);

	/** Resolve TaskId from ObjectiveTag, then increment if the task is Active. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool NotifyObjectiveProgressByTag(const FGameplayTag& ObjectiveTag, int32 Delta = 1);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FExQuestTask> GetActiveQuests() const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FExQuestTask> GetAllQuests() const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FExQuestTask> GetRootQuests() const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FExQuestTask> GetSubQuests(const FGameplayTag& ParentTaskId) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetQuestById(const FGameplayTag& TaskId, FExQuestTask& OutTask) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ResetAllQuests();

	/** Default save: JSON V2; LoadQuestProgress also accepts legacy V1 text */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FString SaveQuestProgress() const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool LoadQuestProgress(const FString& SaveData);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	FString SaveQuestProgressAsJson() const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool LoadQuestProgressFromJson(const FString& JsonSaveData);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	FString SaveQuestProgressAsTextV1() const;

	/** Push runtime state to UExQuestReplicationComponent on authority (server / standalone). */
	void CommitAuthorityReplication();

	/** Apply replicated team state on clients (from GameState replication). */
	void ApplyReplicatedQuestView(UExQuestDataAsset* DefinitionAsset, const FExQuestRuntimeState& RuntimeState);

private:
	UPROPERTY()
	FExQuestData CurrentQuestData;

	UPROPERTY()
	TObjectPtr<UExQuestDataAsset> LoadedQuestAsset;

	TMap<FGameplayTag, EExQuestState> InitialTaskStates;

	bool FindAndUpdateTask(const FGameplayTag& TaskId, TFunctionRef<bool(FExQuestTask&)> UpdateFunc);
	bool LoadQuestProgressLegacyText(const FString& SaveData);

	void CaptureInitialStates();
	void BroadcastTaskStateChange(const FExQuestTask& Task);
	void BroadcastTaskProgress(const FExQuestTask& Task);
	void NotifyQuestDataRefreshed();
	void SyncRuntimeStateCache();

	bool TryUnlockTask(FExQuestTask& Task);
	void UnlockDependentQuests(const FGameplayTag& CompletedTaskId);
	void HandleQuestCompleted(FExQuestTask& Task);
	void TryRollUpParentTasks(const FGameplayTag& CompletedTaskId);
	void ResetTaskObjectives(FExQuestTask& Task);

	bool ApplyObjectiveProgress(FExQuestTask& Task, const FGameplayTag& ObjectiveTag, int32 NewProgress);

	FExQuestRuntimeState CachedRuntimeState;
	bool bApplyingReplicatedView = false;
};
