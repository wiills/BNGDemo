// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Quest/ExQuestTypes.h"
#include "ExQuestDefinition.generated.h"

/**
 * @struct FExQuestObjectiveDefinition
 * @brief 任务目标静态定义（无运行时进度）
 */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestObjectiveDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FGameplayTag ObjectiveId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (ClampMin = "1"))
	int32 TargetProgress = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bIsOptional = false;
};

/**
 * @struct FExQuestTaskDefinition
 * @brief 任务静态定义（无运行时状态）
 */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestTaskDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTag TaskId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText TaskName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	EExQuestState InitialState = EExQuestState::Locked;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FExQuestObjectiveDefinition> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FGameplayTagContainer SubTaskIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FGameplayTagContainer PreTaskIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTag ParentTaskId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bIsRepeatable = false;

	FExQuestTask ToRuntimeTask() const;
};

/**
 * @class UExQuestDataAsset
 * @brief 任务集 DataAsset（策划配置入口）
 */
UCLASS(BlueprintType)
class BLUEPRINTNODEGRAPH_API UExQuestDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FString QuestSetId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText QuestSetName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FExQuestTaskDefinition> TaskDefinitions;

	/** 由静态定义构建可运行的 FExQuestData（目标进度为 0） */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FExQuestData BuildInitialQuestData() const;

#if WITH_EDITOR
	virtual void PostLoad() override;
#endif
};
