// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/FindObjectFlags.h"
#include "ExLatentProxyDefine.generated.h"

/**
 * @struct FExLatentNodeInfo
 * @brief 延迟节点配置信息结构体
 * 
 * 用于存储延迟节点的配置信息，包括唯一标识、日志内容和超时时间等。
 */
USTRUCT(BlueprintType)
struct FExLatentNodeInfo
{
	GENERATED_BODY()

	/** 节点资源UUID */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NodeInfo", meta = (DisplayName = "资源UUID", AllowPrivateAccess = true))
	FString UUID;

	/** 节点唯一ID（UUID + Owner唯一ID） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NodeInfo", meta = (DisplayName = "唯一ID", AllowPrivateAccess = true))
	FString UniqueId;

	/** 开始执行时的日志内容 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NodeInfo", meta = (DisplayName = "开始日志", AllowPrivateAccess = true))
	FString StartLog;

	/** 执行结束时的日志内容 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NodeInfo", meta = (DisplayName = "结束日志", AllowPrivateAccess = true))
	FString EndLog;

	/** 超时时间（秒），0表示不启用超时 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NodeInfo", meta = (DisplayName = "超时时间", AllowPrivateAccess = true, MinValue = 0.0f, ClampMin = "0.0"))
	float TimeOut = 0.f;
};


BLUEPRINTNODEGRAPH_API DECLARE_LOG_CATEGORY_EXTERN(LogBlueprintNodeGraph, Display, All);
BLUEPRINTNODEGRAPH_API DECLARE_LOG_CATEGORY_EXTERN(LogLatentTask, Display, All);
BLUEPRINTNODEGRAPH_API DECLARE_LOG_CATEGORY_EXTERN(LogAsyncAction, Display, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExLatentTaskAsyncDelegate);


namespace ExLatentTaskHelper
{
	/**
	 * 获取枚举值的字符串表示
	 * Example Call:
	 * ExLatentTaskHelper::EnumToString(EExLatentTaskState::Pending);
	 */
	template<typename T>
	static FString EnumToString(T EnumValue)
	{
		const UEnum* EnumPtr = StaticEnum<T>();
		if (!EnumPtr)
			return NSLOCTEXT("Invalid", "Invalid", "Invalid").ToString();
		return EnumPtr->GetNameStringByValue(static_cast<int64>(EnumValue));
	}

	/* Pin Names */
	const FName WorldContextPinName(TEXT("WorldContextObject"));
	const FName ClassPinName(TEXT("Class"));
	const FName ObjectPinName(TEXT("Object"));
	const FName InputCountPinName = TEXT("InputCount");
	const FName UUIDPinName = TEXT("UUID");
	const FName K2NodeInfoPinName = TEXT("K2NodeInfo");
}
