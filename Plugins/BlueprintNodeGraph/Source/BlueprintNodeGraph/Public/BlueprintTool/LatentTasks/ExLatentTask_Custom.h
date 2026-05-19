// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/LatentTasks/ExBase_LatentTask.h"
#include "ExLatentTask_Custom.generated.h"

/**
 * @class UExLatentTask_Custom
 * @brief 用户自定义延迟任务（蓝图子类的推荐父类）
 *
 * 表示流程中的一个可复用任务单元，由 ExK2Node_LatentTaskObject 调用 CreateProxy 创建实例。
 */
UCLASS(Blueprintable, BlueprintType, HideDropdown, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen))
class BLUEPRINTNODEGRAPH_API UExLatentTask_Custom : public UExBase_LatentTask
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "LatentTasks", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "CreateLatentTask"))
	static UExLatentTask_Custom* CreateProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_Custom> Class);
};

/**
 * @class UExLatentTask_BranchSync
 * @brief 多分支同步延迟任务（K2 内部，等待所有输入分支完成）
 */
UCLASS(NotBlueprintable, NotBlueprintType, HideDropdown, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen))
class BLUEPRINTNODEGRAPH_API UExLatentTask_BranchSync : public UExBase_LatentTask
{
	GENERATED_BODY()

	UPROPERTY()
	bool bBranchesFinished = false;

	UPROPERTY()
	FString m_SelfUUID;

	UPROPERTY()
	int32 m_InputBranchCount;

public:
	void SetUUIDAndCount(FString UUID, int32 InCount)
	{
		m_SelfUUID = UUID;
		m_InputBranchCount = InCount;
	}

	virtual bool IsBranchesFinished() const { return bBranchesFinished; }

	UFUNCTION(BlueprintCallable, Category = "ExTasks", meta = (BlueprintInternalUseOnly = "true", DisplayName = "CreateLatentTaskBranchSync", WorldContext = "WorldContextObject", HidePin = "UUID,InputCount"))
	static UExLatentTask_BranchSync* CreateProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_BranchSync> Class, FString UUID, int32 InputCount);

	virtual void Activate() override final
	{
		UE_LOG(LogLatentTask, Display, TEXT("[UExLatentTask_BranchSync::Activate] - %s, Count: %d"), *GetName(), m_InputBranchCount);
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
