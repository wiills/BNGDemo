// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Async/IAsyncTask.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "ExLatentTaskInterface.generated.h"

/**
 * @enum EExLatentTaskState
 * @brief 延迟任务的状态枚�?
 */
UENUM(BlueprintType)
enum class EExLatentTaskState : uint8
{
	/** 任务已取�?*/
	Cancelled,

	/** 任务已完�?*/
	Completed,

	/** 任务执行失败 */
	Failed,

	/** 任务等待执行 */
	Pending,

	/** 任务正在执行 */
	Running,
};

/**
 * @class UExLatentTaskInterface
 * @brief 延迟任务接口（UInterface�?
 * 
 * 用于定义延迟任务的标准接口，不能在蓝图中实现�?
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UExLatentTaskInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @class IExLatentTaskInterface
 * @brief 延迟任务接口实现
 * 
 * 定义延迟任务的核心接口方法，包括状态管理、生命周期控制等�?
 */
class BLUEPRINTNODEGRAPH_API IExLatentTaskInterface
{
	GENERATED_BODY()

public:
	// ========== Context ==========

	/**
	 * @brief 检查是否为本地客户�?
	 * @return 是否为本地客户端
	 */
	virtual bool IsLocal() { return false; }

	// ========== State Management ==========

	/**
	 * @brief 获取任务状�?
	 * @return 当前任务状�?
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual EExLatentTaskState GetState() const = 0;

	/**
	 * @brief 设置任务状�?
	 * @param InState 目标状�?
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual void SetState(EExLatentTaskState InState) = 0;

	/**
	 * @brief 获取状态名称字符串
	 * @return 状态名�?
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual FString GetStateName() const;

	/**
	 * @brief 检查任务是否处于等待状�?
	 * @return 是否等待�?
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual bool IsPending() { return GetState() == EExLatentTaskState::Pending; }

	/**
	 * @brief 检查任务是否正在运�?
	 * @return 是否运行�?
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual bool IsRunning() { return GetState() == EExLatentTaskState::Running; }

	/**
	 * @brief 检查任务是否已停止
	 * @return 是否已停�?
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual bool IsStopped() { return GetState() == EExLatentTaskState::Completed; }

	/**
	 * @brief 检查任务是否已取消
	 * @return 是否已取�?
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual bool IsCancelled() { return GetState() == EExLatentTaskState::Cancelled; }

	// ========== Task Control ==========

	/**
	 * @brief 尝试重置任务
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual void TryReset();

	/**
	 * @brief 尝试启动任务
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual void TryStart();

	/**
	 * @brief 尝试停止任务
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual void TryStop();

	/**
	 * @brief 终止任务（强制取消）
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual void Terminate();

protected:
	// ========== Lifecycle Callbacks ==========

	/**
	 * @brief 启动前回�?
	 */
	UFUNCTION()
	virtual void PreOnStart() {}

	/**
	 * @brief 启动回调
	 */
	UFUNCTION()
	virtual void OnStart() {}

	/**
	 * @brief 停止回调
	 */
	UFUNCTION()
	virtual void OnStop() {}
};
