// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExLatentProxyDefine.h"
#include "UObject/NoExportTypes.h"
#include "ExAsyncActionBase.generated.h"

class UGameInstance;

/**
 * @class UExAsyncActionBase
 * @brief 异步操作基类，为K2节点提供异步任务支持
 *
 * 该类是所有异步操作节点的基类，提供了基础的异步执行框架、
 * 超时机制、GameInstance注册管理等功能。
 *
 * @note 由 ExK2Node_LatentTaskCall 调用
 * @see UExK2Node_LatentTaskCall, FExLatentNodeInfo
 */
UCLASS(Abstract, HideDropdown, Blueprintable, BlueprintType, meta = (SafeHideThen), HideCategories = "NodeInfo")
class BLUEPRINTNODEGRAPH_API UExAsyncActionBase : public UObject
{
	GENERATED_BODY()

public:
	/** 超时计时器句柄 */
	UPROPERTY()
	FTimerHandle m_K2NodeTimerHandle;

protected:
	/** 任务是否已完成 */
	UPROPERTY()
	bool bFinished = false;
	/** 任务是否已初始化 */
	bool binitialized = false;

	/** 节点配置信息，包含UUID、日志、超时时间等 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NodeInfo")
	FExLatentNodeInfo m_NodeInfo;

public:
	/**
	 * @brief 构造函数
	 * @param ObjectInitializer 对象初始化器
	 */
	UExAsyncActionBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * @brief 设置K2节点编辑器配置信息
	 * @param K2NodeInfo 节点配置信息结构体
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	void SetK2NodeInfo(const FExLatentNodeInfo K2NodeInfo);

	/**
	 * @brief 在委托绑定后触发操作执行
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	virtual void Activate();

	/**
	 * @brief 检查任务是否已初始化
	 * @return 任务初始化状态
	 */
	bool IsInitialized() const { return binitialized; }
	
	/**
	 * @brief 检查任务是否已完成
	 * @return 任务完成状态
	 */
	bool IsFinished() const { return bFinished; }

	/**
	 * @brief 设置任务完成状态
	 * @param bCond 是否完成
	 */
	void SetFinished(bool bCond) { bFinished = bCond; }

	/**
	 * @brief 尝试完成任务
	 *
	 * 检查任务状态，如果未完成则标记为完成并调用 OnFinishCall
	 */
	UFUNCTION(BlueprintCallable)
	virtual void TryFinish();

	/**
	 * @brief 获取当前世界上下文
	 * @return 世界对象指针
	 */
	virtual UWorld* GetWorld() const override;

	/**
	 * @brief 将对象注册到GameInstance
	 *
	 * 注册后对象不会被销毁，直到调用 SetReadyToDestroy
	 * 这允许操作在循环中使用或在调用者BP销毁后继续执行
	 *
	 * @param WorldContextObject 世界上下文对象
	 */
	virtual void RegisterWithGameInstance(UObject* WorldContextObject);

	/**
	 * @brief 标记操作完成并允许销毁
	 *
	 * 清除强引用标记，从GameInstance注销对象
	 */
	virtual void SetReadyToDestroy();

	// UObject：BeginDestroy 在基类为 public，override 必须保持 public
	virtual void BeginDestroy() override;

protected:
	/**
	 * @brief 完成回调函数
	 *
	 * 当任务完成时调用，默认实现标记任务为完成状态
	 */
	UFUNCTION()
	virtual void OnFinishCall() {}

	/**
	 * @brief 将对象注册到GameInstance（内部实现）
	 * @param GameInstance 目标GameInstance
	 */
	virtual void RegisterWithGameInstance(UGameInstance* GameInstance);

	/**
	 * @brief 清理定时器
	 */
	void ClearK2NodeTimer();

	/** 注册到的GameInstance弱引用 */
	TWeakObjectPtr<UGameInstance> RegisteredWithGameInstance;
};


/**
 * @brief 异步操作完成委托
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExAsyncActionProxyResult);

/**
 * @class UExAsyncActionProxy
 * @brief 带成功/失败委托的异步操作代理类
 *
 * 继承自 UExAsyncActionBase，添加了成功和失败两个委托，
 * 用于通知调用方操作的执行结果。
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExAsyncActionProxy : public UExAsyncActionBase
{
	GENERATED_BODY()

public:
	/** 操作成功完成时触发 */
	UPROPERTY(BlueprintAssignable, Category = "AsyncAction")
	FOnExAsyncActionProxyResult OnSuccess;

	/** 操作失败时触发 */
	UPROPERTY(BlueprintAssignable, Category = "AsyncAction")
	FOnExAsyncActionProxyResult OnFailure;

	/**
	 * @brief 激活操作（空实现，由子类重写）
	 */
	virtual void Activate() override {}

	/**
	 * @brief 触发成功回调
	 *
	 * 广播 OnSuccess 委托并标记对象可被销毁
	 */
	virtual void DoSuccess() { OnSuccess.Broadcast(); SetReadyToDestroy(); }

	/**
	 * @brief 触发失败回调
	 *
	 * 广播 OnFailure 委托并标记对象可被销毁
	 */
	virtual void DoFail() { OnFailure.Broadcast(); SetReadyToDestroy(); }
};
