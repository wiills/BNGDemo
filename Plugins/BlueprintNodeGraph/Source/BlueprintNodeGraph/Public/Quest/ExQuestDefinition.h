// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Quest/ExQuestTypes.h"
#include "Quest/ExQuestDataImport.h"
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

/**
 * DataTable row struct (one row = one task). Mirrors FExQuestTaskDefinition for CSV / sheet workflows.
 * Create a DataTable with Row Type = FExQuestTaskTableRow, then BuildQuestDataFromTaskTable.
 */
USTRUCT(BlueprintType)
struct BLUEPRINTNODEGRAPH_API FExQuestTaskTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTag TaskId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText TaskName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EExQuestState InitialState = EExQuestState::Locked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestObjectiveDefinition> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer SubTaskIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FGameplayTagContainer PreTaskIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTag ParentTaskId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsRepeatable = false;

	FExQuestTaskDefinition ToTaskDefinition() const;
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

#if WITH_EDITORONLY_DATA
	/** Optional source table for re-import (DT_Quest_* ? paired DA_Quest_*). */
	UPROPERTY(EditAnywhere, Category = "Quest|Import")
	TObjectPtr<UDataTable> SourceTaskTable = nullptr;
#endif

	/** Build FExQuestData with zero objective progress */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	FExQuestData BuildInitialQuestData() const;

	/** Build the same runtime quest data from a task DataTable (FExQuestTaskTableRow). */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	static FExQuestData BuildQuestDataFromTaskTable(
		const UDataTable* TaskTable,
		const FText& InQuestSetName,
		const FString& InQuestSetId = TEXT(""));

#if WITH_EDITOR
	virtual void PostLoad() override;

	UFUNCTION(CallInEditor, Category = "Quest|Import", meta = (DisplayName = "Import From Source Task Table"))
	void EditorImportFromSourceTaskTable();

	/** Replace TaskDefinitions from a FExQuestTaskTableRow DataTable. */
	FExQuestDataImportResult ImportTaskDefinitionsFromDataTable(UDataTable* TaskTable);

	void SetSourceTaskTable(const UDataTable* TaskTable);
#endif
};
