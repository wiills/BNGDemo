// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "ExAsyncAction_LoadAsset.generated.h"

class UAsyncLoadAssetCallbackProxy;

/**
 * @class UExAsyncAction_LoadAsset
 * @brief 异步加载资产的代理类
 * 
 * 提供非阻塞的资产异步加载能力，支�?UObject �?UClass 两种加载模式�?
 * 自动处理资产流送和依赖关系管理�?
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExAsyncAction_LoadAsset : public UExBase_AsyncAction
{
	GENERATED_BODY()

public:
	/** 资产加载完成时触�?*/
	UPROPERTY(BlueprintAssignable, Category = "AsyncAction|AssetLoading")
	FOnExAsyncActionProxyResult OnAssetLoaded;

	/**
	 * @brief 异步加载指定路径的资�?
	 * @param WorldContextObject 世界上下文对�?
	 * @param AssetPath 资产路径（如 "/Game/Path/AssetName.AssetName")
	 * @return 异步操作代理
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", Category = "LatentActions|AssetLoading", BlueprintInternalUseOnly = "true", DisplayName = "Async Load Asset By Path"))
	static UExAsyncAction_LoadAsset* AsyncLoadAssetByPath(
		UObject* WorldContextObject,
		FSoftObjectPath AssetPath
	);

	/**
	 * @brief 异步加载指定类的资产
	 * @param WorldContextObject 世界上下文对�?
	 * @param AssetPath 资产路径
	 * @param AssetClass 资产�?
	 * @return 异步操作代理
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", Category = "LatentActions|AssetLoading", BlueprintInternalUseOnly = "true", DisplayName = "Async Load Asset Class"))
	static UExAsyncAction_LoadAsset* AsyncLoadAssetClass(
		UObject* WorldContextObject,
		FSoftObjectPath AssetPath,
		TSubclassOf<UObject> AssetClass
	);

	virtual void Activate() override;

	/** 异步加载完成（供内部回调与其�?UObject 调用�?*/
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
 * @class UExAsyncAction_LoadAssetCallbackProxy
 * @brief 资产加载回调代理（内部使用）
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExAsyncAction_LoadAssetCallbackProxy : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UExAsyncAction_LoadAsset* ParentProxy;

	FSoftObjectPath AssetPath;
	TSubclassOf<UObject> AssetClass;

	void OnAssetLoaded(FPrimaryAssetId LoadedId);
};
