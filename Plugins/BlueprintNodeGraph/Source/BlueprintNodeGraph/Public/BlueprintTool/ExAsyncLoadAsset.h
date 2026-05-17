// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExAsyncActionBase.h"
#include "ExLatentProxyDefine.h"
#include "ExAsyncLoadAsset.generated.h"

class UAsyncLoadAssetCallbackProxy;

/**
 * @class UExAsyncLoadAsset
 * @brief 异步加载资产的代理类
 * 
 * 提供非阻塞的资产异步加载能力，支持 UObject 和 UClass 两种加载模式。
 * 自动处理资产流送和依赖关系管理。
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExAsyncLoadAsset : public UExAsyncActionBase
{
	GENERATED_BODY()

public:
	/** 资产加载完成时触发 */
	UPROPERTY(BlueprintAssignable, Category = "AsyncAction|AssetLoading")
	FOnExAsyncActionProxyResult OnAssetLoaded;

	/**
	 * @brief 异步加载指定路径的资产
	 * @param WorldContextObject 世界上下文对象
	 * @param AssetPath 资产路径（如 "/Game/Path/AssetName.AssetName")
	 * @return 异步操作代理
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", Category = "LatentActions|AssetLoading", BlueprintInternalUseOnly = "true", DisplayName = "Async Load Asset By Path"))
	static UExAsyncLoadAsset* AsyncLoadAssetByPath(
		UObject* WorldContextObject,
		FSoftObjectPath AssetPath
	);

	/**
	 * @brief 异步加载指定类的资产
	 * @param WorldContextObject 世界上下文对象
	 * @param AssetPath 资产路径
	 * @param AssetClass 资产类
	 * @return 异步操作代理
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", Category = "LatentActions|AssetLoading", BlueprintInternalUseOnly = "true", DisplayName = "Async Load Asset Class"))
	static UExAsyncLoadAsset* AsyncLoadAssetClass(
		UObject* WorldContextObject,
		FSoftObjectPath AssetPath,
		TSubclassOf<UObject> AssetClass
	);

	virtual void Activate() override;

	/** 异步加载完成（供内部回调与其它 UObject 调用） */
	void HandleAssetLoaded();

protected:
	UPROPERTY()
	FSoftObjectPath TargetAssetPath;

	UPROPERTY()
	TSubclassOf<UObject> TargetAssetClass;

	UPROPERTY()
	bool bIsClassLoading = false;
};

/**
 * @class UExAsyncLoadAssetCallbackProxy
 * @brief 资产加载回调代理（内部使用）
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExAsyncLoadAssetCallbackProxy : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UExAsyncLoadAsset* ParentProxy;

	FSoftObjectPath AssetPath;
	TSubclassOf<UObject> AssetClass;

	void OnAssetLoaded(FPrimaryAssetId LoadedId);
};
