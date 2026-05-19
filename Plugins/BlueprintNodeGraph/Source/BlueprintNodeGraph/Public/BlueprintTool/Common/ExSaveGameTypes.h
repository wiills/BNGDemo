// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExSaveGameTypes.generated.h"

/**
 * @struct FExFlowTaskState
 * @brief 单个流程任务的状态数据
 */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExFlowTaskState
{
	GENERATED_BODY()

	/** 任务唯一标识 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	FString TaskId;

	/** 任务类名 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	FString TaskClassName;

	/** 当前状态 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	FString CurrentState;

	/** 任务参数（JSON格式） */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	FString TaskParameters;

	/** 开始时间戳 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	float StartTimestamp = 0.f;

	/** 最后更新时间戳 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	float LastUpdateTimestamp = 0.f;

	/** 是否完成 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	bool bIsCompleted = false;

	/** 完成时间戳 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	float CompletedTimestamp = 0.f;

	/** 任务内检查点索引 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	int32 CheckpointIndex = 0;
};

/**
 * @struct FExFlowState
 * @brief 流程状态数据，用于存档和断点续跑
 */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExFlowState
{
	GENERATED_BODY()

	/** 存档唯一标识 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	FString SaveId;

	/** 存档名称 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	FString SaveName;

	/** 存档创建时间 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	FDateTime CreatedTime;

	/** 存档更新时间 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	FDateTime UpdatedTime;

	/** 流程名称 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	FString FlowName;

	/** 当前任务列表 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	TArray<FExFlowTaskState> ActiveTasks;

	/** 已完成任务列表 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	TArray<FExFlowTaskState> CompletedTasks;

	/** 流程变量（键值对） */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	TMap<FString, FString> FlowVariables;

	/** 检查点索引 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	int32 CheckpointIndex = 0;

	/** 元数据 */
	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	TMap<FString, FString> Metadata;
};
