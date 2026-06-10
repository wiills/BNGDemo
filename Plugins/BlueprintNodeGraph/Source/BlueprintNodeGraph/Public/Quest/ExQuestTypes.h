// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BlueprintTool/LatentTasks/ExLatentTask_Quest.h"
#include "ExQuestTypes.generated.h"

class UExLatentTask_Quest;

/** 任务生命周期状态。Quest lifecycle states. */
UENUM(BlueprintType)
enum class EExQuestState : uint8
{
	Inactive,  // 已解锁，待激活
	Active,    // 进行中
	Completed, // 已完成
	Failed,    // 已失败
	Locked     // 未解锁
};

/** 通用的 LatentTask 参数配置 Payload。Common payload for latent task configuration. */
/** 单个 Objective 运行时数据（进度与定义在 FExQuestData 中合并）。Single objective runtime row. */
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
	int32 CurrentProgress = 0; // 当前进度

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 TargetProgress = 1; // 目标进度（完成阈值）

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EExQuestState State = EExQuestState::Locked; // Objective 状态

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsOptional = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bUIVisible = true;

	/** 目标节点专用的任务树条目样式；为空时 UI 会回退使用所属父任务的 EntryViewClass，保证旧配置继续按任务样式显示。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|UI", meta = (AdvancedDisplay, ToolTip = "目标节点专用的任务树条目样式；未配置时使用所属父任务的 EntryViewClass。"))
	TSoftClassPtr<UUserWidget> EntryViewClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Execution", meta = (ToolTip = "Optional. When empty, objective waits for external progress (NotifyByTag / Route API)."))
	TSubclassOf<UExLatentTask_Quest> LatentTaskClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Execution", meta = (AdvancedDisplay, ToolTip = "Payload passed to the objective latent task instance on creation."))
	FExQuestLatentTaskPayload LatentTaskPayload;

	FExQuestObjective()
		: CurrentProgress(0)
		, TargetProgress(1)
		, State(EExQuestState::Locked)
		, bIsOptional(false)
		, bUIVisible(true)
	{
	}

	bool IsCompleted() const { return State == EExQuestState::Completed; }
	bool CanActivate() const { return State == EExQuestState::Inactive; }
	bool CanUnlock() const { return State == EExQuestState::Locked; }
	void ApplyProgressToState();
};

/** 单条 Task 运行时数据。Task 为汇总容器，完成由 Objectives + SubTasks 驱动。Single quest task runtime row. */
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
	EExQuestState State = EExQuestState::Locked; // Task 状态

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestObjective> Objectives; // 同 Task 内步骤清单（非子 Task）

	/** 任务树：父行填写，列出子 TaskId。Hierarchy child task ids (authored on parent row). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTagContainer SubTaskIds;

	/** 流程前置：RebuildIndices 从 NextTaskIds 反推，勿手填。Derived activate gate; do not author. */
	UPROPERTY(BlueprintReadOnly, meta = (Hidden, ToolTip = "Auto-derived from NextTaskIds at RebuildIndices."))
	FGameplayTagContainer PreTaskIds;

	/** 流程后继：本 Task 完成后尝试激活的下游 TaskId。Flow edges unlocked on complete. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTagContainer NextTaskIds;

	/** 运行时由父行 SubTaskIds 推导；策划通常留空。Derived from parent SubTaskIds at RebuildIndices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest", AdvancedDisplay))
	FGameplayTag ParentTaskId;

	/** Optional task-specific quest entry view. Assign a widget blueprint derived from QuestEntryView when a task needs its own item layout. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|UI", meta = (AdvancedDisplay, ToolTip = "Optional task-specific quest entry view. Assign a widget blueprint derived from QuestEntryView when a task needs its own item layout."))
	TSoftClassPtr<UUserWidget> EntryViewClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Execution", meta = (AdvancedDisplay, ToolTip = "Optional. Default leave empty: task completes when Objectives and SubTasks are done."))
	TSubclassOf<UExLatentTask_Quest> LatentTaskClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Execution", meta = (AdvancedDisplay))
	FExQuestLatentTaskPayload LatentTaskPayload;

	FExQuestTask()
		: State(EExQuestState::Locked)
		, bIsRepeatable(false)
	{
	}

	bool CanActivate() const;
	bool CanUnlock() const;
	bool ArePreTasksSatisfied(const FExQuestData& QuestData) const;
	/** 全部必填 Objective 已完成。All required objectives done. */
	bool IsFullyCompleted() const;
	/** SubTaskIds 中每个子 Task 均为 Completed。Every listed sub-task is completed. */
	bool AreAllSubTasksCompleted(const FExQuestData& QuestData) const;
	/** 自动 Complete 门槛：Objective 满足且子 Task 全完。Auto-complete gate for task rollup. */
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
	EExQuestState State = EExQuestState::Locked;
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

/** 任务集运行时视图：定义 + 进度合并。Flat quest set with merged definition and runtime. */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", Meta = (IgnoreForMemberInitializationTest))
	FString QuestSetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText QuestSetName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestTask> AllTasks;

	FExQuestData()
		: QuestSetId(FGuid::NewGuid().ToString())
	{
	}

	/** 重建索引：ParentTaskId、PreTaskIds、Objective 反查表等。Rebuild lookup indices and derived graph fields. */
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
	TMap<FGameplayTag, int32> TaskIdToIndex; // TaskId → AllTasks 下标
	TMap<FGameplayTag, int32> ObjectiveTagToTaskIndex; // ObjectiveTag → 所属 Task 下标
	TMap<FGameplayTag, TArray<int32>> ParentTaskIdToChildIndices; // 父 TaskId → 子 Task 下标列表
	TMap<FGameplayTag, TArray<int32>> PreTaskIdToDependentIndices; // 前置 TaskId → 依赖它的 Task 下标列表

	bool FindTaskInList(const TArray<FExQuestTask>& Tasks, const FGameplayTag& TaskId, FExQuestTask& OutTask) const;
};
