// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExAsyncActionBase.h"
#include "ExSubsystemGetter.h"
#include "Kismet/GameplayStatics.h"
#include "ExLatentActionManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDelayCompletedDelegate);

/**
 * @class UExLatentActionProxy
 * @brief 等待所有输入分支完成后执行输出的异步代理类
 */
UCLASS(Abstract, HideDropdown, Blueprintable, BlueprintType, meta = (SafeHideThen))
class BLUEPRINTNODEGRAPH_API UExLatentActionProxy : public UExAsyncActionBase
{
	GENERATED_BODY()

	/** 是否所有分支都已完成 */
	UPROPERTY()
	bool bBranchesFinished = false;

protected:
	/** 节点唯一标识符 */
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

	UExLatentActionProxy(const FObjectInitializer& ObjectInitializer)
	{
		if (!HasAnyFlags(RF_ClassDefaultObject))
		{
			SetFlags(RF_StrongRefOnFrame);
		}
	}

	virtual bool IsBranchesFinished() const { return bBranchesFinished; }

	virtual void Activate() override final
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[UExLatentActionProxy::Activate] - %s, Count: %d"), *GetName(), m_InputBranchCount);
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
 * @class UExLatentActionProxyBase
 * @brief 等待所有输入分支完成后执行输出的基础异步代理类
 */
UCLASS(Abstract, HideDropdown, NotBlueprintable, NotBlueprintType, meta = (HideThen), HideCategories = "NodeInfo")
class BLUEPRINTNODEGRAPH_API UExLatentActionProxyBase : public UObject
{
	GENERATED_BODY()

protected:
	/** 任务是否已完成 */
	UPROPERTY()
	bool bFinished = false;
	/** 任务是否已初始化 */
	UPROPERTY()
	bool bInitialized = false;

	/** 是否所有分支都已完成 */
	UPROPERTY()
	bool bBranchesFinished = false;

	/** 超时计时器句柄 */
	UPROPERTY()
	FTimerHandle m_K2NodeTimerHandle;

	/** 节点配置信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NodeInfo")
	FExLatentNodeInfo m_NodeInfo;

	/** 节点唯一标识符 */
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

	UExLatentActionProxyBase(const FObjectInitializer& ObjectInitializer)
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
	 * @brief 设置K2节点编辑器配置信息
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
	 * @brief 在委托绑定后触发操作执行（默认视为该输入分支成功完成）
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
	/** 多输入分支中的单次报告：默认实现等价于旧版递减 InputCount；UExWaitBranchProxy 等子类可覆盖以实现 All/Any/Count */
	virtual void HandleBranchReported(bool bSuccess);
	
};

/**
 * @class UExLatentActionManager
 * @brief 延迟动作实例管理器
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExLatentActionManager : public UGameInstanceSubsystem, public FExSubsystemGetter<UExLatentActionManager>
{
	GENERATED_BODY()

public:
	UExLatentActionManager() {}

	virtual void Deinitialize() override
	{
		ProxyMap.Empty();
	}

	void SetProxyObject(const FString& Key, UObject* Proxy)
	{
		if (!ProxyMap.Contains(Key))
		{
			ProxyMap.Add(Key, Proxy);
		}
	}

	void RemoveProxyObject(const FString& Key)
	{
		if (ProxyMap.Contains(Key))
		{
			ProxyMap.Remove(Key);
		}
	}

	template<class T>
	T* GetProxyObject(const FString& Key)
	{
		if (const auto Ptr = ProxyMap.Find(Key))
		{
			return Cast<T>(*Ptr);
		}
		return nullptr;
	}

private:
	UPROPERTY()
	TMap<FString, UObject*> ProxyMap;
};

template <class T>
T* CreateWaitProxyCall(UObject* WorldContextObject, FString UUID, int32 InputCount)
{
	const auto WaitInputManager = UExLatentActionManager::Get();
	if (!WaitInputManager)
	{
		return nullptr;
	}
	auto ObjectUUID = FString::Printf(TEXT("%u%s"), GetTypeHash(WorldContextObject), *UUID);
	auto Proxy = WaitInputManager->GetProxyObject<T>(ObjectUUID);
	if (!Proxy)
	{
		Proxy = NewObject<T>(WorldContextObject);
		Proxy->SetUUIDAndCount(ObjectUUID, InputCount);
		WaitInputManager->SetProxyObject(ObjectUUID, Proxy);
	}
	UE_LOG(LogAsyncAction, Display, TEXT("[CreateWaitProxyCall] - Proxy: %s, WorldContextObject: %s, ObjectUUID: %s, InputCount: %d"),
		*Proxy->GetName(), IsValid(WorldContextObject) ? *WorldContextObject->GetName() : TEXT(""), *ObjectUUID, InputCount);
	return Proxy;
}

template <class T>
T* CreateWaitProxyCallWithClass(UObject* WorldContextObject, UClass* TargetClass, FString UUID, int32 InputCount)
{
	const auto WaitInputManager = UExLatentActionManager::Get();
	if (!WaitInputManager)
	{
		return nullptr;
	}
	auto ObjectUUID = FString::Printf(TEXT("%u%s"), GetTypeHash(WorldContextObject), *UUID);
	auto Proxy = WaitInputManager->GetProxyObject<T>(ObjectUUID);
	if (!Proxy)
	{
		Proxy = Cast<T>(UGameplayStatics::SpawnObject(TargetClass, WorldContextObject));
		Proxy->SetUUIDAndCount(ObjectUUID, InputCount);
		WaitInputManager->SetProxyObject(ObjectUUID, Proxy);
	}
	UE_LOG(LogAsyncAction, Display, TEXT("[CreateWaitProxyCall] - Proxy: %s, WorldContextObject: %s, ObjectUUID: %s, InputCount: %d"),
		*Proxy->GetName(), IsValid(WorldContextObject) ? *WorldContextObject->GetName() : TEXT(""), *ObjectUUID, InputCount);
	return Proxy;
}
