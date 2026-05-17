// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

class UWorldPartition;
struct FWorldPartitionStreamingStatus;

#include "ExWorldPartitionSubsystem.generated.h"

// 作为 UFUNCTION 形参时必须使用 DECLARE_DYNAMIC_DELEGATE_*，不能用 MULTICAST。
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnLevelLoaded, FName, LevelName);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnLevelUnloaded, FName, LevelName);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnLevelLoadFailed, FName, LevelName, FString, ErrorMessage);

/**
 * @class UExWorldPartitionSubsystem
 * @brief WorldPartition 子关卡管理系统
 * 
 * 提供子关卡的异步加载/卸载能力，支持延迟任务和异步代理两种调用方式。
 * 自动处理 WorldPartition 上下文和流送依赖关系。
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExWorldPartitionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UExWorldPartitionSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * @brief 异步加载子关卡
	 * @param LevelName 子关卡名称
	 * @param bLoadAsync 是否异步加载
	 * @param CompletionDelegate 加载完成回调
	 * @param FailureDelegate 加载失败回调
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldPartition|Latent", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	void StreamLevel(
		UObject* WorldContextObject,
		FName LevelName,
		bool bLoadAsync,
		FOnLevelLoaded CompletionDelegate,
		FOnLevelLoadFailed FailureDelegate
	);

	/**
	 * @brief 异步卸载子关卡
	 * @param LevelName 子关卡名称
	 * @param bUnloadAsync 是否异步卸载
	 * @param CompletionDelegate 卸载完成回调
	 */
	UFUNCTION(BlueprintCallable, Category = "WorldPartition|Latent", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	void UnstreamLevel(
		UObject* WorldContextObject,
		FName LevelName,
		bool bUnloadAsync,
		FOnLevelUnloaded CompletionDelegate
	);

	/**
	 * @brief 检查子关卡是否已加载
	 * @param LevelName 子关卡名称
	 * @return 是否已加载
	 */
	UFUNCTION(BlueprintPure, Category = "WorldPartition|Query")
	bool IsLevelLoaded(FName LevelName) const;

	/**
	 * @brief 获取已加载的子关卡列表
	 * @return 已加载关卡名称数组
	 */
	UFUNCTION(BlueprintPure, Category = "WorldPartition|Query")
	TArray<FName> GetLoadedLevels() const;

protected:
	UPROPERTY()
	TArray<FName> LoadedLevels;

	TMap<FName, TArray<FOnLevelLoaded>> PendingLoadDelegates;
	TMap<FName, TArray<FOnLevelLoadFailed>> PendingLoadFailedDelegates;
	TMap<FName, TArray<FOnLevelUnloaded>> PendingUnloadDelegates;

	void OnLevelLoadingStarted(const FName& LevelName);
	void OnLevelLoadingCompleted(const FName& LevelName);
	void OnLevelLoadingStatusChanged(const UWorldPartition* WorldPartition, const FWorldPartitionStreamingStatus& StreamingStatus);
};
