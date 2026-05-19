// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ExQuestTypes.generated.h"

/**
 * @enum EExQuestState
 * @brief 任务状态枚举
 */
UENUM(BlueprintType)
enum class EExQuestState : uint8
{
	Inactive,
	Active,
	Completed,
	Failed,
	Locked
};

/**
 * @struct FExQuestObjective
 * @brief 任务目标数据
 */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestObjective
{
	GENERATED_BODY()

	/** 目标唯一ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag ObjectiveId;

	/** 目标描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;

	/** 当前进度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 CurrentProgress = 0;

	/** 目标进度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 TargetProgress = 1;

	/** 是否完成 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsCompleted = false;

	/** 是否可选 */
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

/**
 * @struct FExQuestTask
 * @brief 任务数据
 */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestTask
{
	GENERATED_BODY()

	/** 任务唯一ID（使用GameplayTag） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTag TaskId;

	/** 任务名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText TaskName;

	/** 任务描述 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;

	/** 任务状态 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EExQuestState State = EExQuestState::Locked;

	/** 任务目标列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestObjective> Objectives;

	/** 子任务ID列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer SubTaskIds;

	/** 前置任务ID列表 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer PreTaskIds;

	/** 父任务ID（如果是子任务） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTag ParentTaskId;

	/** 是否可重复 */
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
	bool IsFullyCompleted() const;
	float GetCompletionPercent() const;
};

/**
 * @struct FExQuestObjectiveRuntime
 * @brief 目标运行时进度（可单独存档）
 */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestObjectiveRuntime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTag ObjectiveId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsCompleted = false;
};

/**
 * @struct FExQuestTaskRuntime
 * @brief 任务运行时状态（可单独存档）
 */
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

/**
 * @struct FExQuestRuntimeState
 * @brief 任务集运行时状态（与 DataAsset 定义分离）
 */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestRuntimeState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString QuestSetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestTaskRuntime> TaskStates;
};

/**
 * @struct FExQuestData
 * @brief 完整的任务数据（定义 + 运行时合并视图）
 */
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
	bool FindTaskIdByObjectiveId(const FGameplayTag& ObjectiveId, FGameplayTag& OutTaskId) const;
	bool CanActivateTask(const FGameplayTag& TaskId) const;
	TArray<FExQuestTask> GetAllActiveTasks() const;
	TArray<FExQuestTask> GetAllCompletedTasks() const;
	TArray<FExQuestTask> GetRootTasks() const;
	TArray<FExQuestTask> GetSubTasks(const FGameplayTag& ParentTaskId) const;
	TArray<FGameplayTag> GetTaskIdsWithPreTask(const FGameplayTag& PreTaskId) const;

	FExQuestRuntimeState ExtractRuntimeState() const;
	void ApplyRuntimeState(const FExQuestRuntimeState& RuntimeState);

private:
	TMap<FGameplayTag, int32> TaskIdToIndex;
	TMap<FGameplayTag, int32> ObjectiveIdToTaskIndex;
	TMap<FGameplayTag, TArray<int32>> ParentTaskIdToChildIndices;
	TMap<FGameplayTag, TArray<int32>> PreTaskIdToDependentIndices;

	bool FindTaskInList(const TArray<FExQuestTask>& Tasks, const FGameplayTag& TaskId, FExQuestTask& OutTask) const;
};
