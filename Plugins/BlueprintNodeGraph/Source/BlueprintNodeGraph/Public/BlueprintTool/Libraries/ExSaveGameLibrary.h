// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/Common/ExSaveGameTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ExSaveGameLibrary.generated.h"

class USaveGame;

/**
 * @class UExSaveGameLibrary
 * @brief 存档功能库
 * 
 * 提供流程存档和断点续跑的核心功能，包括保存、加载、删除等操作。
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExSaveGameLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief 保存流程状态到存档
	 * @param WorldContextObject 世界上下文对象
	 * @param SaveId 存档唯一标识
	 * @param SaveName 存档名称
	 * @param FlowName 流程名称
	 * @param TaskStates 任务状态数组
	 * @param FlowVariables 流程变量映射
	 * @return 是否保存成功
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame|Flow", meta = (WorldContext = "WorldContextObject"))
	static bool SaveFlowState(
		UObject* WorldContextObject,
		const FString& SaveId,
		const FString& SaveName,
		const FString& FlowName,
		const TArray<FExFlowTaskState>& TaskStates,
		const TMap<FString, FString>& FlowVariables
	);

	/**
	 * @brief 加载流程状态
	 * @param WorldContextObject 世界上下文对象
	 * @param SaveId 存档唯一标识
	 * @return 流程状态数据
	 */
	UFUNCTION(BlueprintPure, Category = "SaveGame|Flow", meta = (WorldContext = "WorldContextObject"))
	static FExFlowState LoadFlowState(
		UObject* WorldContextObject,
		const FString& SaveId
	);

	/**
	 * @brief 获取所有存档列表
	 * @param WorldContextObject 世界上下文对象
	 * @return 存档列表
	 */
	UFUNCTION(BlueprintPure, Category = "SaveGame|Flow", meta = (WorldContext = "WorldContextObject"))
	static TArray<FExFlowState> GetAllFlowSaves(
		UObject* WorldContextObject
	);

	/**
	 * @brief 删除指定存档
	 * @param WorldContextObject 世界上下文对象
	 * @param SaveId 存档唯一标识
	 * @return 是否删除成功
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame|Flow", meta = (WorldContext = "WorldContextObject"))
	static bool DeleteFlowSave(
		UObject* WorldContextObject,
		const FString& SaveId
	);

	/**
	 * @brief 检查存档是否存在
	 * @param WorldContextObject 世界上下文对象
	 * @param SaveId 存档唯一标识
	 * @return 是否存在
	 */
	UFUNCTION(BlueprintPure, Category = "SaveGame|Flow", meta = (WorldContext = "WorldContextObject"))
	static bool FlowSaveExists(
		UObject* WorldContextObject,
		const FString& SaveId
	);

	/**
	 * @brief 序列化任务状态到 JSON
	 * @param TaskState 任务状态
	 * @return JSON 字符串
	 */
	UFUNCTION(BlueprintPure, Category = "SaveGame|Utilities")
	static FString SerializeTaskState(const FExFlowTaskState& TaskState);

	/**
	 * @brief 从 JSON 反序列化任务状态
	 * @param JsonString JSON 字符串
	 * @return 任务状态
	 */
	UFUNCTION(BlueprintPure, Category = "SaveGame|Utilities")
	static FExFlowTaskState DeserializeTaskState(const FString& JsonString);
};
