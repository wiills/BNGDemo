// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExQuestTypes.h"
#include "ExQuestManagerSubsystem.generated.h"

class UExQuestDataAsset;
class UExLatentTask_Quest;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStateChanged, const FExQuestTask&, QuestTask);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestObjectiveUpdated, const FExQuestObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestProgressChanged, const FGameplayTag&, TaskId, float, CompletionPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestDataLoaded);

/** 任务运行时管理器：状态机、自动完成链、LatentTask 生命周期。Game-instance quest manager. */
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

	/** 从 DataAsset 加载；QuestSetId 一致时可保留运行时进度。Load from DA; optionally preserve runtime progress. */
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

	/** 将 Task 设为 Active 并启动执行（需满足 PreTaskIds）。Activate task when prerequisites are met. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool ActivateQuest(const FGameplayTag& TaskId);

	/** Locked → Inactive（前置满足时）。Unlock task when prerequisites are met. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool UnlockQuest(const FGameplayTag& TaskId);

	/** 仅 Active Task 可 Complete；触发自动完成链。Complete an active task and run completion chain. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool CompleteQuest(const FGameplayTag& TaskId);

	/** 测试专用：跳过 Active 门槛强制完成。Debug only: force-complete regardless of state. */
	UFUNCTION(BlueprintCallable, Category = "Quest|Debug", meta = (ToolTip = "Test only. Completes the task without requiring Active state. Does not run in shipping by default usage."))
	bool ForceCompleteQuest(const FGameplayTag& TaskId);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool FailQuest(const FGameplayTag& TaskId);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool UpdateQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag, int32 NewProgress);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IncrementQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag, int32 Delta = 1);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool CompleteQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag);

	/** 按 ObjectiveTag 反查 Task 并递增进度（Task 须 Active）。Resolve task by objective tag and increment progress. */
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

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RebuildActiveLatentTasks();

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

	/** 权威端将运行时状态推送到 GameState 复制组件。Push runtime state to replication on authority. */
	void CommitAuthorityReplication();

	/** 客户端应用复制视图（不 Spawn LatentTask）。Apply replicated view on clients. */
	void ApplyReplicatedQuestView(UExQuestDataAsset* DefinitionAsset, const FExQuestRuntimeState& RuntimeState);

private:
	UPROPERTY()
	FExQuestData CurrentQuestData; // 当前任务集（定义 + 运行时合并）

	UPROPERTY()
	TObjectPtr<UExQuestDataAsset> LoadedQuestAsset; // 最近一次 Load 的 DA

	TMap<FGameplayTag, EExQuestState> InitialTaskStates; // Load/Reset 时捕获，供 ResetAllQuests 恢复

	bool FindAndUpdateTask(const FGameplayTag& TaskId, TFunctionRef<bool(FExQuestTask&)> UpdateFunc);
	bool LoadQuestProgressLegacyText(const FString& SaveData);

	void CaptureInitialStates();
	void BroadcastTaskStateChange(const FExQuestTask& Task);
	void BroadcastTaskProgress(const FExQuestTask& Task);
	void NotifyQuestDataRefreshed();
	void SyncRuntimeStateCache();

	bool TryUnlockTask(FExQuestTask& Task);
	void UnlockDependentQuests(const FGameplayTag& CompletedTaskId);

	/** 任务完成后的内部链：NextTask 扇出、SubTask 串行、父 Task 冒泡。Internal completion chain; not exposed to Blueprint. */
	void HandleQuestCompleted(FExQuestTask& Task);

	/** 子 Task 完成时，若父 Task 已满足 IsReadyToComplete 则自动 Complete 并继续冒泡。Roll up to parent when ready. */
	void TryRollUpParentTasks(const FGameplayTag& CompletedTaskId);

	void ResetTaskObjectives(FExQuestTask& Task);

	/** 若尚无 Active Objective，解锁列表中第一个未完成的 Objective。Unlock first pending objective only. */
	void ActivateTaskObjectives(FExQuestTask& Task);

	/** Load/Reset 后：对所有 Active Task 解锁首个 Objective 并 Spawn LatentTask。Post-load execution bootstrap. */
	void InitializeActiveTaskExecution();

	/** 为 Active Task Spawn Task 级与全部 Active Objective 级 LatentTask（幂等）。Start latent drivers for active task/objectives. */
	void StartLatentTasksForActiveTask(const FGameplayTag& TaskId);

	void CreateLatentTaskForQuest(const FGameplayTag& TaskId);
	void DestroyLatentTaskForQuest(const FGameplayTag& TaskId);
	void DestroyAllLatentTasks();

	/** 同一父下串行激活第一个 Locked/Inactive 子 Task；已 Active/Completed 的跳过。Activate next sibling sub-task only. */
	void ActivateNextSubTask(const FGameplayTag& ParentTaskId);

	void CreateLatentTaskForObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag);
	void DestroyLatentTaskForObjective(const FGameplayTag& ObjectiveTag);
	void DestroyObjectiveLatentTasksForQuest(const FGameplayTag& TaskId);

	/** 写回 Objective 进度；满足 IsReadyToComplete 时自动 Complete Task。Apply objective progress and auto-complete task. */
	bool ApplyObjectiveProgress(FExQuestTask& Task, const FGameplayTag& ObjectiveTag, int32 NewProgress);

	FExQuestRuntimeState CachedRuntimeState; // 复制用快照
	bool bApplyingReplicatedView = false; // 防止复制回写再次 Publish

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UExLatentTask_Quest>> ActiveLatentTasks; // TaskId → Task 级 Latent（可选）

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UExLatentTask_Quest>> ActiveObjectiveLatentTasks; // ObjectiveTag → Objective 级 Latent
};
