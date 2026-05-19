// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ExQuestTypes.h"
#include "ExQuestBlueprintLibrary.generated.h"

/**
 * @class UExQuestBlueprintLibrary
 * @brief 任务系统蓝图函数库
 *
 * 提供蓝图友好的任务系统操作接口
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExQuestBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ========== 创建类 ==========

	/**
	 * 创建示例任务数据（用于测试）
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Create")
	static FExQuestData CreateExampleQuestData();

	/**
	 * 创建任务目标
	 * @param ObjectiveId 目标ID
	 * @param Description 目标描述
	 * @param TargetProgress 目标进度
	 * @param bIsOptional 是否可选
	 * @return 任务目标对象
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Create")
	static FExQuestObjective CreateQuestObjective(
		const FGameplayTag& ObjectiveId,
		const FText& Description,
		int32 TargetProgress = 1,
		bool bIsOptional = false);

	/**
	 * 创建任务
	 * @param TaskId 任务ID
	 * @param TaskName 任务名称
	 * @param Description 任务描述
	 * @param InitialState 初始状态
	 * @return 任务对象
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Create")
	static FExQuestTask CreateQuestTask(
		const FGameplayTag& TaskId,
		const FText& TaskName,
		const FText& Description,
		EExQuestState InitialState = EExQuestState::Locked);

	/**
	 * 创建任务数据
	 * @param QuestSetName 任务集名称
	 * @param AllTasks 所有任务列表
	 * @return 任务数据
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Create")
	static FExQuestData CreateQuestData(
		const FText& QuestSetName,
		const TArray<FExQuestTask>& AllTasks);

	// ========== 修改类 ==========

	/**
	 * 给任务添加子任务ID
	 * @param Task 任务
	 * @param SubTaskId 子任务ID
	 * @return 修改后的任务
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Modify")
	static FExQuestTask AddSubTaskId(FExQuestTask Task, const FGameplayTag& SubTaskId);

	/**
	 * 给任务添加前置任务ID
	 * @param Task 任务
	 * @param PreTaskId 前置任务ID
	 * @return 修改后的任务
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Modify")
	static FExQuestTask AddPreTaskId(FExQuestTask Task, const FGameplayTag& PreTaskId);

	/**
	 * 给任务添加目标
	 * @param Task 任务
	 * @param Objective 目标
	 * @return 修改后的任务
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Modify")
	static FExQuestTask AddObjectiveToQuest(FExQuestTask Task, const FExQuestObjective& Objective);

	// ========== 查询类 ==========

	/**
	 * 从任务数据中查找任务
	 * @param QuestData 任务数据
	 * @param TaskId 任务ID
	 * @param OutTask 输出任务
	 * @return 是否找到
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|Query")
	static bool FindQuestInData(const FExQuestData& QuestData, const FGameplayTag& TaskId, FExQuestTask& OutTask);

	/**
	 * 获取根任务列表
	 * @param QuestData 任务数据
	 * @return 根任务列表
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|Query")
	static TArray<FExQuestTask> GetRootQuestsInData(const FExQuestData& QuestData);

	/**
	 * 获取子任务列表
	 * @param QuestData 任务数据
	 * @param ParentTaskId 父任务ID
	 * @return 子任务列表
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|Query")
	static TArray<FExQuestTask> GetSubQuestsInData(const FExQuestData& QuestData, const FGameplayTag& ParentTaskId);

	/**
	 * 获取任务数据中所有激活的任务
	 * @param QuestData 任务数据
	 * @return 激活的任务列表
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|Query")
	static TArray<FExQuestTask> GetAllActiveQuestsInData(const FExQuestData& QuestData);

	/**
	 * 获取任务数据中所有完成的任务
	 * @param QuestData 任务数据
	 * @return 完成的任务列表
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|Query")
	static TArray<FExQuestTask> GetAllCompletedQuestsInData(const FExQuestData& QuestData);

	// ========== 状态类 ==========

	/**
	 * 获取任务完成百分比
	 * @param Task 任务
	 * @return 完成百分比（0.0 - 100.0）
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static float GetQuestCompletionPercent(const FExQuestTask& Task);

	/**
	 * 检查任务是否完全完成（包括所有目标）
	 * @param Task 任务
	 * @return 是否完全完成
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool IsQuestFullyCompleted(const FExQuestTask& Task);

	/**
	 * 检查任务是否可激活
	 * @param Task 任务
	 * @return 是否可激活
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool CanQuestActivate(const FExQuestTask& Task);

	/**
	 * 检查任务是否锁定
	 * @param Task 任务
	 * @return 是否锁定
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool IsQuestLocked(const FExQuestTask& Task);

	/**
	 * 检查任务是否失败
	 * @param Task 任务
	 * @return 是否失败
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool IsQuestFailed(const FExQuestTask& Task);

	/**
	 * 检查任务是否未激活
	 * @param Task 任务
	 * @return 是否未激活
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool IsQuestInactive(const FExQuestTask& Task);

	/**
	 * 获取任务状态的显示文本
	 * @param State 任务状态
	 * @return 状态文本
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static FText GetQuestStateText(EExQuestState State);

	/**
	 * 获取任务状态的颜色
	 * @param State 任务状态
	 * @return 状态颜色
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static FLinearColor GetQuestStateColor(EExQuestState State);

	// ========== 辅助类 ==========

	/**
	 * 获取任务管理器（从游戏实例）
	 * @param WorldContextObject 世界上下文对象
	 * @return 任务管理器子系统
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|Helper", meta = (WorldContext = "WorldContextObject"))
	static class UExQuestManagerSubsystem* GetQuestManager(UObject* WorldContextObject);

	/**
	 * 从字符串创建GameplayTag
	 * @param TagString Tag字符串
	 * @return GameplayTag
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|Helper")
	static FGameplayTag MakeQuestTag(const FString& TagString);

	/**
	 * 检查任务是否有子任务
	 * @param Task 任务
	 * @return 是否有子任务
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|Helper")
	static bool HasSubTasks(const FExQuestTask& Task);

	/**
	 * 检查任务是否有前置任务
	 * @param Task 任务
	 * @return 是否有前置任务
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|Helper")
	static bool HasPreTasks(const FExQuestTask& Task);

	/**
	 * 获取任务数据中任务总数
	 * @param QuestData 任务数据
	 * @return 任务总数
	 */
	UFUNCTION(BlueprintPure, Category = "Quest System|Helper")
	static int32 GetQuestCount(const FExQuestData& QuestData);
};
