// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExLatentTaskBase.h"
#include "ExLatentTaskProxy.generated.h"

/**
 * @class UExLatentTaskProxy
 * @brief 创建延迟任务的代理类
 * 
 * 由 ExK2Node_LatentTaskObject 调用，用于创建延迟任务实例。
 */
UCLASS(HideDropdown, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen))
class BLUEPRINTNODEGRAPH_API UExLatentTaskProxy : public UExLatentTaskBase
{
	GENERATED_BODY()

public:
	/**
	 * @brief 创建延迟任务
	 * @param WorldContextObject 世界上下文对象
	 * @param Class 要创建的任务类
	 * @return 创建的任务实例
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTasks", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "CreateLatentTask"))
	static UExLatentTaskProxy* CreateProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTaskProxy> Class);
};

/**
 * @class UExLatentTaskUUIDProxy
 * @brief 带UUID唯一标识的延迟任务代理类
 * 
 * 支持多分支同步，等待所有输入分支完成后执行输出。
 */
UCLASS(HideDropdown, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen))
class BLUEPRINTNODEGRAPH_API UExLatentTaskUUIDProxy : public UExLatentTaskBase
{
	GENERATED_BODY()

	/** 是否所有分支都已完成 */
	UPROPERTY()
	bool bBranchesFinished = false;

	/** 节点唯一标识符（用于GC防护） */
	UPROPERTY()
	FString m_SelfUUID;

	/** 输入分支计数 */
	UPROPERTY()
	int32 m_InputBranchCount;

public:
	void SetUUIDAndCount(FString UUID, int32 InCount)
	{
		m_SelfUUID = UUID;
		m_InputBranchCount = InCount;
	}

	virtual bool IsBranchesFinished() const { return bBranchesFinished; }

	/**
	 * @brief 创建带UUID的延迟任务
	 * @param WorldContextObject 世界上下文对象
	 * @param Class 要创建的任务类
	 * @param UUID 唯一标识符
	 * @param InputCount 输入分支数量
	 * @return 创建的任务实例
	 */
	UFUNCTION(BlueprintCallable, Category = "ExTasks", meta = (BlueprintInternalUseOnly = "true", DisplayName = "CreateLatentTaskUUID", WorldContext = "WorldContextObject", HidePin = "UUID,InputCount"))
	static UExLatentTaskUUIDProxy* CreateProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTaskUUIDProxy> Class, FString UUID, int32 InputCount);

	/**
	 * @brief 在委托绑定后触发操作执行
	 */
	virtual void Activate() override final
	{
		UE_LOG(LogLatentTask, Display, TEXT("[UExLatentTaskUUIDProxy::Activate] - %s, Count: %d"), *GetName(), m_InputBranchCount);
		m_InputBranchCount--;
		if (m_InputBranchCount <= 0 && !IsStopped())
		{
			bBranchesFinished = true;
			OnBranchesFinished();
			TryStop();
		}
	}

protected:
	virtual void OnBranchesFinished() {}
	virtual void OnStop() override;
	void RemoveWaitInstance() const;
};
