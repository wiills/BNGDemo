// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ExQuestTypes.generated.h"

/** Quest lifecycle state */
UENUM(BlueprintType)
enum class EExQuestState : uint8
{
	Inactive,
	Active,
	Completed,
	Failed,
	Locked
};

/** Single objective with progress (runtime + definition merged in FExQuestData) */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestObjective
{
	GENERATED_BODY()

	/** Objective configuration GameplayTag (not a world instance id) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag ObjectiveTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 TargetProgress = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsOptional = false;

	FExQuestObjective()
		: CurrentProgress(0)
		, TargetProgress(1)
		, bIsCompleted(false)
		, bIsOptional(false)
	{
	}
};

/** Single quest task */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestTask
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTag TaskId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText TaskName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EExQuestState State = EExQuestState::Locked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer SubTaskIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer PreTaskIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTag ParentTaskId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsRepeatable = false;

	FExQuestTask()
		: State(EExQuestState::Locked)
		, bIsRepeatable(false)
	{
	}

	bool CanActivate() const;
	bool CanUnlock() const;
	bool ArePreTasksSatisfied(const FExQuestData& QuestData) const;
	/** All required objectives on this task are done. */
	bool IsFullyCompleted() const;
	/** Every entry in SubTaskIds exists and is Completed. */
	bool AreAllSubTasksCompleted(const FExQuestData& QuestData) const;
	/** Objectives satisfied and all listed sub-tasks completed (auto-complete gate). */
	bool IsReadyToComplete(const FExQuestData& QuestData) const;
	float GetCompletionPercent() const;
	/** Objectives + sub-task completion for UI progress. */
	float GetAggregateCompletionPercent(const FExQuestData& QuestData) const;
};

/** Runtime objective progress (save-friendly) */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestObjectiveRuntime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag ObjectiveTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsCompleted = false;
};

/** Runtime task state (save-friendly) */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestTaskRuntime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag TaskId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EExQuestState State = EExQuestState::Locked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestObjectiveRuntime> Objectives;
};

/** Runtime state for a quest set (separate from UExQuestDataAsset definitions) */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString QuestSetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestTaskRuntime> TaskStates;
};

/** Flat quest set: definition + runtime merged view */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString QuestSetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText QuestSetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestTask> AllTasks;

	FExQuestData()
		: QuestSetId(FGuid::NewGuid().ToString())
	{
	}

	void RebuildIndices();
	int32 FindTaskIndex(const FGameplayTag& TaskId) const;
	bool FindTaskById(const FGameplayTag& TaskId, FExQuestTask& OutTask) const;
	bool FindMutableTaskById(const FGameplayTag& TaskId, FExQuestTask*& OutTask);
	bool FindTaskIdByObjectiveTag(const FGameplayTag& ObjectiveTag, FGameplayTag& OutTaskId) const;
	bool CanActivateTask(const FGameplayTag& TaskId) const;
	TArray<FExQuestTask> GetAllActiveTasks() const;
	TArray<FExQuestTask> GetAllCompletedTasks() const;
	TArray<FExQuestTask> GetRootTasks() const;
	TArray<FExQuestTask> GetSubTasks(const FGameplayTag& ParentTaskId) const;
	TArray<FGameplayTag> GetTaskIdsWithPreTask(const FGameplayTag& PreTaskId) const;

	FExQuestRuntimeState ExtractRuntimeState() const;
	void ApplyRuntimeState(const FExQuestRuntimeState& RuntimeState);

	/** Fill empty task/objective text and targets from an authored definition snapshot. */
	void EnrichMetadataFrom(const FExQuestData& DefinitionData);

private:
	TMap<FGameplayTag, int32> TaskIdToIndex;
	TMap<FGameplayTag, int32> ObjectiveTagToTaskIndex;
	TMap<FGameplayTag, TArray<int32>> ParentTaskIdToChildIndices;
	TMap<FGameplayTag, TArray<int32>> PreTaskIdToDependentIndices;

	bool FindTaskInList(const TArray<FExQuestTask>& Tasks, const FGameplayTag& TaskId, FExQuestTask& OutTask) const;
};
