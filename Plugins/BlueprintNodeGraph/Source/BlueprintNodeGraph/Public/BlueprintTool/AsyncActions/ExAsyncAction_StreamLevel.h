// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "ExAsyncAction_StreamLevel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelLoadedDelegate, FName, LevelName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelLoadFailedDelegate, FName, LevelName, FString, ErrorMessage);

/**
 * @class UExAsyncAction_StreamLevel
 * @brief 异步加载/卸载关卡的代理类
 * 
 * 提供非阻塞的关卡流送能力，支持加载和卸载两种操作模式。
 * 自动处理 WorldPartition 上下文和依赖关系管理。
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExAsyncAction_StreamLevel : public UExBase_AsyncAction
{
	GENERATED_BODY()

public:
	/** 关卡加载完成时触发 */
	UPROPERTY(BlueprintAssignable, Category = "AsyncAction|LevelStreaming")
	FOnLevelLoadedDelegate OnLevelLoaded;

	/** 关卡加载失败时触发 */
	UPROPERTY(BlueprintAssignable, Category = "AsyncAction|LevelStreaming")
	FOnLevelLoadFailedDelegate OnLevelLoadFailed;

	/** 关卡卸载完成时触发 */
	UPROPERTY(BlueprintAssignable, Category = "AsyncAction|LevelStreaming")
	FOnLevelLoadedDelegate OnLevelUnloaded;

	/**
	 * @brief 异步加载关卡
	 * @param WorldContextObject 世界上下文对象
	 * @param LevelName 关卡名称
	 * @param bMakeVisibleAfterLoad 加载后是否自动显示
	 * @param bShouldBlockOnSlowLoading 是否在慢加载时阻塞
	 * @return 异步操作代理
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", Category = "LatentActions|LevelStreaming", BlueprintInternalUseOnly = "true", DisplayName = "Async Load Level"))
	static UExAsyncAction_StreamLevel* AsyncLoadLevel(
		UObject* WorldContextObject,
		FName LevelName,
		bool bMakeVisibleAfterLoad = true,
		bool bShouldBlockOnSlowLoading = false
	);

	/**
	 * @brief 异步卸载关卡
	 * @param WorldContextObject 世界上下文对象
	 * @param LevelName 关卡名称
	 * @return 异步操作代理
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", Category = "LatentActions|LevelStreaming", BlueprintInternalUseOnly = "true", DisplayName = "Async Unload Level"))
	static UExAsyncAction_StreamLevel* AsyncUnloadLevel(
		UObject* WorldContextObject,
		FName LevelName
	);

	virtual void Activate() override;

protected:
	UPROPERTY()
	FName TargetLevelName;

	UPROPERTY()
	bool bIsLoading = true;

	UPROPERTY()
	bool bMakeVisibleAfterLoad = true;

	UPROPERTY()
	bool bShouldBlockOnSlowLoading = false;

	void HandleLevelLoaded(FName LevelName);
	void HandleLevelLoadFailed(FName LevelName, const FString& ErrorMessage);
	void HandleLevelUnloaded(FName LevelName);
};
