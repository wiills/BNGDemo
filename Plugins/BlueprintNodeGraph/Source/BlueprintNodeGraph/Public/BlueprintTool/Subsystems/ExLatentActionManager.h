// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/Proxies/ExBase_FlowProxy.h"
#include "BlueprintTool/Common/ExSubsystemGetter.h"
#include "Kismet/GameplayStatics.h"
#include "ExLatentActionManager.generated.h"

/**
 * @class UExLatentActionManager
 * @brief 延迟动作实例管理�?
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
