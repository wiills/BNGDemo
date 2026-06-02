// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Quest/ExQuestTypes.h"
#include "Quest/ExQuestDataImport.h"
#include "ExQuestDefinition.generated.h"

class UExLatentTask_Quest;

/** 静态 Objective 定义（无运行时进度）。Static objective definition without runtime progress. */
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
	EExQuestState InitialState = EExQuestState::Locked;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bIsOptional = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bUIVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Execution", meta = (ToolTip = "Optional. When empty, objective stays Active and waits for external progress (NotifyByTag / Route API)."))
	TSubclassOf<UExLatentTask_Quest> LatentTaskClass;

	/** 目标节点专用的任务树条目样式；为空表示沿用父任务 EntryViewClass，便于只给少数目标做特殊表现。 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|UI", meta = (AdvancedDisplay, ToolTip = "目标节点专用的任务树条目样式；未配置时使用父任务 EntryViewClass。"))
	TSoftClassPtr<UUserWidget> EntryViewClass;
};

/** 静态 Task 定义（无运行时 State/进度）。Static task definition without runtime state. */
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
	EExQuestState InitialState = EExQuestState::Locked; // Load 后的初始 Task 状态

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	TArray<FExQuestObjectiveDefinition> Objectives; // 步骤清单；日常玩法挂 Objective LatentTask

	/** 任务树子节点（父行填写）。Child tasks in hierarchy; authored on parent row. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTagContainer SubTaskIds;

	UPROPERTY(BlueprintReadOnly, meta = (Hidden, ToolTip = "Auto-derived from NextTaskIds at RebuildIndices. Do not edit."))
	FGameplayTagContainer PreTaskIds; // 由 NextTaskIds 反推，编辑器 Hidden

	/** 本 Task 完成后激活的下游 Task。Downstream tasks to activate after this one completes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (Categories = "Quest", ToolTip = "Forward edges: tasks to unlock after this one completes."))
	FGameplayTagContainer NextTaskIds;

	/** Optional legacy field; RebuildIndices derives this from parent row SubTaskIds at load time. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest", meta = (Categories = "Quest", AdvancedDisplay, Tooltip = "Normally leave empty. Parent link is derived from the parent task's SubTaskIds when indices are rebuilt."))
	FGameplayTag ParentTaskId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Execution", meta = (AdvancedDisplay, ToolTip = "Optional. Default leave empty: task completes when Objectives and SubTasks are done. Use only when this task has no Objective list and one latent drives the whole task."))
	TSubclassOf<UExLatentTask_Quest> LatentTaskClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|UI", meta = (AdvancedDisplay, ToolTip = "Optional task-specific quest entry view. Assign a widget blueprint derived from QuestEntryView when this task needs a dedicated UI."))
	TSoftClassPtr<UUserWidget> EntryViewClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	bool bIsRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest|Execution", meta = (AdvancedDisplay, EditCondition = "LatentTaskClass != nullptr", ToolTip = "Payload passed to the latent task instance on creation."))
	FInstancedStruct LatentTaskPayload;

	FExQuestTask ToRuntimeTask() const;
};

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
	EExQuestState InitialState = EExQuestState::Locked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TArray<FExQuestObjectiveDefinition> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest"))
	FGameplayTagContainer SubTaskIds;

	UPROPERTY(BlueprintReadOnly, meta = (Hidden, ToolTip = "Auto-derived from NextTaskIds at RebuildIndices. Do not edit."))
	FGameplayTagContainer PreTaskIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest", ToolTip = "Forward edges: tasks to unlock after this one completes."))
	FGameplayTagContainer NextTaskIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|UI", meta = (AdvancedDisplay, ToolTip = "Optional task-specific quest entry view. Assign a widget blueprint derived from QuestEntryView when this task needs a dedicated UI."))
	TSoftClassPtr<UUserWidget> EntryViewClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool bIsRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Execution", meta = (AdvancedDisplay, ToolTip = "Optional. Default leave empty: task completes when Objectives and SubTasks are done. Use only when this task has no Objective list and one latent drives the whole task."))
	TSubclassOf<UExLatentTask_Quest> LatentTaskClass;

	FExQuestTaskDefinition ToTaskDefinition() const;
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
	TArray<FExQuestTaskDefinition> TaskDefinitions;

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
