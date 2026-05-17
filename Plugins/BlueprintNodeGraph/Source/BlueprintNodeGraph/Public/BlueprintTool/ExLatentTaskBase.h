// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExLatentProxyDefine.h"
#include "ExLatentTaskInterface.h"
#include "ExLatentTaskBase.generated.h"

/**
 * @class UExLatentTaskBase
 * @brief 延迟任务基类
 * 
 * 提供基础的延迟任务功能，支持超时控制、网络复制、状态管理等。
 */
UCLASS(Abstract, HideDropdown, Blueprintable, BlueprintType)
class BLUEPRINTNODEGRAPH_API UExLatentTaskBase : public UObject, public IExLatentTaskInterface
{
	GENERATED_BODY()

public:
	/** 任务开始时触发 */
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnStart"))
	FOnExLatentTaskAsyncDelegate StartDelegate;

	/** 任务完成时触发 */
	UPROPERTY(BlueprintAssignable, meta = (DisplayName = "OnComplete"))
	FOnExLatentTaskAsyncDelegate CompleteDelegate;

protected:
	/** 节点配置信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NodeInfo")
	FExLatentNodeInfo NodeInfo;

	/** 超时定时器句柄 */
	UPROPERTY()
	FTimerHandle m_TimeoutTimerHandle;

public:
	/**
	 * @brief 构造函数
	 * @param ObjectInitializer 对象初始化器
	 */
	UExLatentTaskBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * @brief 获取开始委托
	 * @return 开始委托引用
	 */
	FOnExLatentTaskAsyncDelegate& GetStartDelegate() { return StartDelegate; }

	/**
	 * @brief 获取完成委托
	 * @return 完成委托引用
	 */
	FOnExLatentTaskAsyncDelegate& GetCompleteDelegate() { return CompleteDelegate; }

	// ========== Network ==========
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, ReplicatedUsing = OnRep_RunningState)
	EExLatentTaskState RunningState = EExLatentTaskState::Pending;

	UFUNCTION()
	void OnRep_RunningState();

	virtual bool IsSupportedForNetworking() const override { return true; }
	// ========== Network ==========

	// ========== State Management ==========
	virtual EExLatentTaskState GetState() const override { return RunningState; }
	virtual void SetState(EExLatentTaskState InState) override { RunningState = InState; }
	// ========== State Management ==========

	// ========== Factory Function ==========
	/**
	 * @brief 创建延迟任务
	 * @param WorldContextObject 世界上下文对象
	 * @param Class 要创建的任务类
	 * @return 创建的任务实例
	 */
	UFUNCTION(BlueprintCallable, Category = "LatentTasks", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Create Latent Task"))
	static UExLatentTaskBase* CreateTask(UObject* WorldContextObject, TSubclassOf<UExLatentTaskBase> Class);

	/**
	 * @brief 设置K2节点编辑器配置信息
	 * @param K2NodeInfo 节点配置信息
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	void SetK2NodeInfo(const FExLatentNodeInfo K2NodeInfo);

	/**
	 * @brief 在委托绑定后触发操作执行
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	virtual void Activate();
	// ========== Factory Function ==========

	// ========== Context ==========
	virtual bool IsLocal() override;
	virtual UWorld* GetWorld() const override;
	// ========== Context ==========

	// ========== Task Interface ==========
	/** 任务开始回调（蓝图实现） */
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveOnStart();

	/** 任务停止回调（蓝图实现） */
	UFUNCTION(BlueprintImplementableEvent)
	void ReceiveOnStop();

	/**
	 * @brief 终止任务
	 */
	virtual void Terminate() override;
	// ========== Task Interface ==========

	// UObject：BeginDestroy 在基类为 public，override 必须保持 public，不能收窄可见性
	virtual void BeginDestroy() override;

protected:
	/**
	 * @brief 任务开始
	 */
	virtual void OnStart() override;

	/**
	 * @brief 任务停止
	 */
	virtual void OnStop() override;

	/**
	 * @brief 清理定时器
	 */
	void ClearTimeoutTimer();
};
