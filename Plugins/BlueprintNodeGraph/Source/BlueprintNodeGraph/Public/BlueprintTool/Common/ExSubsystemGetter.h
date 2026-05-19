// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

/**
 * @class FExSubsystemGetter
 * @brief BlueprintNodeGraph 插件�?Subsystem 访问�?
 * 
 * 用于获取 UGameInstanceSubsystem、UWorldSubsystem 的单例实例�?
 */
template<class T>
class FExSubsystemGetter
{
public:
	FExSubsystemGetter() = default;

	T* operator()() const { return Get(); }

	/**
	 * @brief 获取世界上下�?
	 * @return 当前世界指针
	 */
	static UWorld* StaticGetWorld()
	{
		if (UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			return World;
		}
		
		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (WorldContext.World() && WorldContext.World()->GetGameInstance())
			{
				return WorldContext.World();
			}
		}
		
		return nullptr;
	}

	/**
	 * @brief 获取 Subsystem 实例
	 * @return Subsystem 对象指针，失败返�?nullptr
	 */
	static T* Get()
	{
		if (!IsValid(StaticGetWorld()))
		{
			return nullptr;
		}
		if (!StaticGetWorld()->GetGameInstance())
		{
			return nullptr;
		}

		if constexpr (std::is_base_of_v<UGameInstanceSubsystem, T>)
		{
			return StaticGetWorld()->GetGameInstance()->GetSubsystem<T>();
		}
		if constexpr (std::is_base_of_v<UWorldSubsystem, T>)
		{
			return StaticGetWorld()->GetSubsystem<T>();
		}

		checkf(false, TEXT("Type Not Supported in FExSubsystemGetter::Get() - Only UGameInstanceSubsystem and UWorldSubsystem are supported"));
		return nullptr;
	}

private:
	T* GetObject() { return static_cast<T*>(this); }
};
