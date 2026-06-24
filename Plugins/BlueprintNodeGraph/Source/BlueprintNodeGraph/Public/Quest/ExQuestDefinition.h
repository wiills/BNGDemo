// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Quest/ExQuestTypes.h"
#include "Quest/ExQuestDataImport.h"
#include "ExQuestDefinition.generated.h"

class UExLatentTask_Quest;

/**
 * DataTable 行结构（一行 = 一条 Task）。
 * 父子关系仅在父行 SubTaskIds 填写；PreTaskIds 由 NextTaskIds 自动反推。
 * DataTable row: one task per row; parent links via SubTaskIds on parent row only.
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
	FGameplayTag ContextID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	EExQuestState InitialState = EExQuestState::Locked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTagContainer SubTaskIds;

	UPROPERTY(BlueprintReadOnly, meta = (Hidden))
	FGameplayTagContainer PreTaskIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTagContainer NextTaskIds;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsRepeatable = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FExQuestUIConfig UIConfig;

	FExQuestTask ToRuntimeTask() const;
};

/** 策划任务集 DataAsset（运行时 Load 入口）。Authored quest set loaded at runtime. */
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
	TArray<FExQuestTask> TaskDefinitions;

#if WITH_EDITORONLY_DATA
	/** Optional source table for re-import (DT_Quest_* ? paired DA_Quest_*). */
	UPROPERTY(EditAnywhere, Category = "Quest|Import")
	TObjectPtr<UDataTable> SourceTaskTable = nullptr;
#endif

	/** 构建初始 FExQuestData（Objective 进度为 0）。Build quest data with zero objective progress. */
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
