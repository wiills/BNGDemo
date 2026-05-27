// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/LatentTasks/ExBase_LatentTask.h"
#include "BlueprintTool/Common/ExSaveGameTypes.h"
#include "ExLatentTask_Saveable.generated.h"


/**
 * @class UExLatentTask_Saveable
 * @brief 支持存档的延迟任务基�?
 * 
 * 提供基础的存档和断点续跑能力�?
 * 子类可以重写 SerializeState �?DeserializeState 来实现自定义的序列化逻辑�?
 * 需要存�?断点续跑时的蓝图父类�?
 */
UCLASS(Abstract, HideDropdown, Blueprintable, BlueprintType)
class BLUEPRINTNODEGRAPH_API UExLatentTask_Saveable : public UExBase_LatentTask
{
	GENERATED_BODY()

public:
	/**
	 * @brief 序列化任务状�?
	 * @return 任务状态数据结�?
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask|Saveable")
	virtual FExFlowTaskState SerializeState();

	/**
	 * @brief 反序列化任务状�?
	 * @param State 任务状态数�?
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask|Saveable")
	virtual void DeserializeState(const FExFlowTaskState& State);

	/**
	 * @brief 获取检查点数据
	 * @return 检查点数据
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask|Saveable")
	FString GetCheckpointData();

	/**
	 * @brief 从检查点恢复
	 * @param CheckpointData 检查点数据
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask|Saveable")
	void RestoreFromCheckpoint(const FString& CheckpointData);

protected:
	/** 是否启用自动存档 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saveable")
	bool bAutoSave = true;

	/** 存档间隔（秒�?*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Saveable", meta = (EditCondition = "bAutoSave"))
	float AutoSaveInterval = 60.f;

	/** 检查点索引 */
	UPROPERTY()
	int32 CurrentCheckpointIndex = 0;

	/** 上次存档时间�?*/
	UPROPERTY()
	float LastSaveTimestamp = 0.f;

	/**
	 * @brief 子类重写：获取任务特定数�?
	 * @return 任务特定数据�?JSON 字符�?
	 */
	virtual FString GetTaskSpecificData() { return TEXT("{}"); }

	/**
	 * @brief 子类重写：从任务特定数据恢复
	 * @param Data JSON 字符�?
	 */
	virtual void RestoreTaskSpecificData(const FString& Data) {}

	/**
	 * @brief 检查是否应该自动存�?
	 */
	void CheckAutoSave();

	/**
	 * @brief 标记检查点
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask|Saveable")
	void MarkCheckpoint();
};
