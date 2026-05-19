// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExQuestTypes.h"
#include "ExQuestManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStateChanged, const FExQuestTask&, QuestTask);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestObjectiveUpdated, const FExQuestObjective&, Objective);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestProgressChanged, const FGameplayTag&, TaskId, float, CompletionPercent);

/**
 * @class UExQuestManagerSubsystem
 * @brief 任务管理子系统
 *
 * 管理游戏中的所有任务，包括任务的激活、完成、进度更新等功能
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExQuestManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 任务状态变化委托 */
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStateChanged OnQuestStateChanged;

	/** 任务目标更新委托 */
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestObjectiveUpdated OnQuestObjectiveUpdated;

	/** 任务进度变化委托 */
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestProgressChanged OnQuestProgressChanged;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 加载任务数据 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void LoadQuestData(const FExQuestData& QuestData);

	/** 获取当前任务数据 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	const FExQuestData& GetQuestData() const { return CurrentQuestData; }

	/** 激活任务 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool ActivateQuest(const FGameplayTag& TaskId);

	/** 完成任务 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool CompleteQuest(const FGameplayTag& TaskId);

	/** 失败任务 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool FailQuest(const FGameplayTag& TaskId);

	/** 更新任务目标进度 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool UpdateQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveId, int32 NewProgress);

	/** 完成任务目标 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool CompleteQuestObjective(const FGameplayTag& TaskId, const FGameplayTag& ObjectiveId);

	/** 获取激活的任务列表 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FExQuestTask> GetActiveQuests() const;

	/** 获取所有任务 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FExQuestTask> GetAllQuests() const;

	/** 获取根任务列表 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FExQuestTask> GetRootQuests() const;

	/** 获取子任务列表 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	TArray<FExQuestTask> GetSubQuests(const FGameplayTag& ParentTaskId) const;

	/** 根据ID获取任务 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetQuestById(const FGameplayTag& TaskId, FExQuestTask& OutTask) const;

	/** 重置所有任务 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ResetAllQuests();

	/** 保存任务进度 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FString SaveQuestProgress() const;

	/** 加载任务进度 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool LoadQuestProgress(const FString& SaveData);

private:
	/** 当前任务数据 */
	UPROPERTY()
	FExQuestData CurrentQuestData;

	/** 递归查找并更新任务 */
	bool FindAndUpdateTask(TArray<FExQuestTask>& Tasks, const FGameplayTag& TaskId, TFunctionRef<bool(FExQuestTask&)> UpdateFunc);
};
