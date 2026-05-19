// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Quest/ExQuestTypes.h"
#include "ExQuestDefinition.generated.h"

/** Static objective definition (no runtime progress) */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestObjectiveDefinition
{
	GENERATED_BODY()

	/** Register in DefaultGameplayTags.ini under Quest.* */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTag ObjectiveTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (ClampMin = "1"))
	int32 TargetProgress = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bIsOptional = false;
};

/** Static task definition (no runtime state) */
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

/** Quest set authored as a DataAsset */
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

	/** Build FExQuestData with zero objective progress */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FExQuestData BuildInitialQuestData() const;

#if WITH_EDITOR
	virtual void PostLoad() override;
#endif
};
