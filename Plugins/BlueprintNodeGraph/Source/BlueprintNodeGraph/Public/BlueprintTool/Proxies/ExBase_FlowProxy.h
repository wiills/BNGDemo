// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "ExBase_FlowProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDelayCompletedDelegate);

/**
 * @class UExAsyncAction_BranchSync
 * @brief 等待所有输入分支完成后执行输出的异步代理类
 */
UCLASS(Abstract, HideDropdown, Blueprintable, BlueprintType, meta = (SafeHideThen))
class BLUEPRINTNODEGRAPH_API UExAsyncAction_BranchSync : public UExBase_AsyncAction
{
	GENERATED_BODY()

	/** 是否所有分支都已完�?*/
	UPROPERTY()
	bool bBranchesFinished = false;

protected:
	/** 节点唯一标识�?*/
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

	UExAsyncAction_BranchSync(const FObjectInitializer& ObjectInitializer)
	{
		if (!HasAnyFlags(RF_ClassDefaultObject))
		{
			SetFlags(RF_StrongRefOnFrame);
		}
	}

	virtual bool IsBranchesFinished() const { return bBranchesFinished; }

	virtual void Activate() override final
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[UExAsyncAction_BranchSync::Activate] - %s, Count: %d"), *GetName(), m_InputBranchCount);
		m_InputBranchCount--;
		if (m_InputBranchCount <= 0 && !IsFinished())
		{
			bBranchesFinished = true;
			OnBranchesFinished();
			TryFinish();
		}
	}

	virtual void TryFinish() override;

protected:
	virtual void OnBranchesFinished() {}
	virtual void OnFinishCall() override;
	void RemoveWaitInstance();
};

/**
 * @class UExBase_FlowProxy
 * @brief 等待所有输入分支完成后执行输出的基础异步代理�?
 */
UCLASS(Abstract, HideDropdown, NotBlueprintable, NotBlueprintType, meta = (HideThen), HideCategories = "NodeInfo")
class BLUEPRINTNODEGRAPH_API UExBase_FlowProxy : public UObject
{
	GENERATED_BODY()

protected:
	/** 任务是否已完�?*/
	UPROPERTY()
	bool bFinished = false;
	/** 任务是否已初始化 */
	UPROPERTY()
	bool bInitialized = false;

	/** 是否所有分支都已完�?*/
	UPROPERTY()
	bool bBranchesFinished = false;

	/** 超时计时器句�?*/
	UPROPERTY()
	FTimerHandle m_K2NodeTimerHandle;

	/** 节点配置信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NodeInfo")
	FExLatentNodeInfo m_NodeInfo;

	/** 节点唯一标识�?*/
	UPROPERTY()
	FString m_SelfUUID;

	/** 输入分支计数 */
	UPROPERTY()
	int32 m_ConstInputBranchCount;
	UPROPERTY()
	int32 m_NeedSuccessBranchCount = 0;

	/** 完成委托 */
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnCompleted"))
	FOnDelayCompletedDelegate m_OnCompletedDelegate;

private:
	UPROPERTY()
	int32 m_CurrentSuccessBranchCount = 0;
	
public:
	void SetUUIDAndCount(FString UUID, int32 InCount)
	{
		m_SelfUUID = UUID;
		m_ConstInputBranchCount = InCount;
		m_NeedSuccessBranchCount = m_ConstInputBranchCount;
	}

	UExBase_FlowProxy(const FObjectInitializer& ObjectInitializer)
	{
		if (!HasAnyFlags(RF_ClassDefaultObject))
		{
			SetFlags(RF_StrongRefOnFrame);
		}
	}
	
	void SetNeedBranchCount(int32 InCount)
	{
		m_NeedSuccessBranchCount = InCount;
	}

	/**
	 * @brief 设置K2节点编辑器配置信�?
	 * @param K2NodeInfo 节点配置信息
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	void SetK2NodeInfo(const FExLatentNodeInfo K2NodeInfo);

	/** 任务是否已初始化 */
	bool IsInitialized() const { return bInitialized; }
	bool IsFinished() const { return bFinished; }
	void SetFinished(bool bCond) { bFinished = bCond; }

	UFUNCTION(BlueprintCallable)
	void TryFinish();

	/**
	 * @brief 在委托绑定后触发操作执行（默认视为该输入分支成功完成�?
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	void Activate();

	virtual void BeginDestroy() override;

protected:
	bool CheckBranchesFinished() const { return m_CurrentSuccessBranchCount >= m_NeedSuccessBranchCount; }
	bool IsBranchesFinished() const { return bBranchesFinished; }
	virtual bool IsRemoveAfterBranches() const { return true; }
	virtual bool IsCustomFinish() const { return false; }

	virtual void OnOneBranchFinished() {}
	virtual void OnBranchesFinished() {}
	UFUNCTION()
	virtual void OnFinishCall();
	void RemoveWaitInstance();
	
private:
	/** 多输入分支中的单次报告：默认实现等价于旧版递减 InputCount；UExProxy_WaitBranch 等子类可覆盖以实�?All/Any/Count */
	virtual void HandleBranchReported(bool bSuccess);
	
};

