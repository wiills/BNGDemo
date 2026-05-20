// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ExQuestTypes.h"
#include "Quest/ExQuestMessageTypes.h"
#include "ExQuestBlueprintLibrary.generated.h"

class UExQuestDataAsset;
class UExQuestReplicationComponent;

/** Blueprint helpers for the quest system */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExQuestBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Quest System|Create")
	static FExQuestData CreateExampleQuestData();

	UFUNCTION(BlueprintCallable, Category = "Quest System|Create")
	static FExQuestObjective CreateQuestObjective(
		const FGameplayTag& ObjectiveTag,
		const FText& Description,
		int32 TargetProgress = 1,
		bool bIsOptional = false);

	/** Builds FExQuestTask definition data (not the Quest Task latent node). */
	UFUNCTION(BlueprintPure, Category = "Quest System|Create", meta = (DisplayName = "Make Quest Task Data", ToolTip = "Builds quest task struct for arrays or DataAsset authoring. For runtime latent flow use the Quest Task graph node."))
	static FExQuestTask MakeQuestTaskData(
		const FGameplayTag& TaskId,
		const FText& TaskName,
		const FText& Description,
		EExQuestState InitialState = EExQuestState::Locked);

	UFUNCTION(BlueprintCallable, Category = "Quest System|Create")
	static FExQuestData CreateQuestData(
		const FText& QuestSetName,
		const TArray<FExQuestTask>& AllTasks);

	UFUNCTION(BlueprintCallable, Category = "Quest System|Modify")
	static FExQuestTask AddSubTaskId(FExQuestTask Task, const FGameplayTag& SubTaskId);

	UFUNCTION(BlueprintCallable, Category = "Quest System|Modify")
	static FExQuestTask AddPreTaskId(FExQuestTask Task, const FGameplayTag& PreTaskId);

	UFUNCTION(BlueprintCallable, Category = "Quest System|Modify")
	static FExQuestTask AddObjectiveToQuest(FExQuestTask Task, const FExQuestObjective& Objective);

	UFUNCTION(BlueprintPure, Category = "Quest System|Query")
	static bool FindQuestInData(const FExQuestData& QuestData, const FGameplayTag& TaskId, FExQuestTask& OutTask);

	UFUNCTION(BlueprintPure, Category = "Quest System|Query")
	static TArray<FExQuestTask> GetRootQuestsInData(const FExQuestData& QuestData);

	UFUNCTION(BlueprintPure, Category = "Quest System|Query")
	static TArray<FExQuestTask> GetSubQuestsInData(const FExQuestData& QuestData, const FGameplayTag& ParentTaskId);

	UFUNCTION(BlueprintPure, Category = "Quest System|Query")
	static TArray<FExQuestTask> GetAllActiveQuestsInData(const FExQuestData& QuestData);

	UFUNCTION(BlueprintPure, Category = "Quest System|Query")
	static TArray<FExQuestTask> GetAllCompletedQuestsInData(const FExQuestData& QuestData);

	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static float GetQuestCompletionPercent(const FExQuestTask& Task);

	/** Objectives + sub-tasks (for parent tasks with SubTaskIds). */
	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static float GetQuestAggregateCompletionPercentWithData(const FExQuestData& QuestData, const FExQuestTask& Task);

	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool IsQuestFullyCompleted(const FExQuestTask& Task);

	/** Objectives done and all SubTaskIds completed (matches auto-complete / rollup). */
	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool IsQuestReadyToCompleteWithData(const FExQuestData& QuestData, const FExQuestTask& Task);

	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool CanQuestUnlockWithData(const FExQuestData& QuestData, const FGameplayTag& TaskId);

	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool CanQuestActivate(const FExQuestTask& Task);

	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool CanQuestActivateWithData(const FExQuestData& QuestData, const FGameplayTag& TaskId);

	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool IsQuestLocked(const FExQuestTask& Task);

	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool IsQuestFailed(const FExQuestTask& Task);

	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static bool IsQuestInactive(const FExQuestTask& Task);

	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static FText GetQuestStateText(EExQuestState State);

	UFUNCTION(BlueprintPure, Category = "Quest System|State")
	static FLinearColor GetQuestStateColor(EExQuestState State);

	UFUNCTION(BlueprintPure, Category = "Quest System|Helper", meta = (WorldContext = "WorldContextObject"))
	static class UExQuestManagerSubsystem* GetQuestManager(UObject* WorldContextObject);

	/** Authority / Standalone: add UExQuestReplicationComponent to GameState when missing. */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Network", meta = (WorldContext = "WorldContextObject"))
	static class UExQuestReplicationComponent* EnsureQuestReplicationOnGameState(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Quest System|Helper", meta = (WorldContext = "WorldContextObject"))
	static bool UnlockQuest(UObject* WorldContextObject, const FGameplayTag& TaskId);

	UFUNCTION(BlueprintCallable, Category = "Quest System|Helper", meta = (WorldContext = "WorldContextObject"))
	static bool ActivateQuest(UObject* WorldContextObject, const FGameplayTag& TaskId);

	/** Locked -> Inactive -> Active when possible (Quest Task OnStart uses this). */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Helper", meta = (WorldContext = "WorldContextObject"))
	static bool EnsureQuestActive(UObject* WorldContextObject, const FGameplayTag& TaskId);

	UFUNCTION(BlueprintCallable, Category = "Quest System|Helper", meta = (WorldContext = "WorldContextObject"))
	static bool IncrementQuestObjective(UObject* WorldContextObject, const FGameplayTag& TaskId, const FGameplayTag& ObjectiveTag, int32 Delta = 1);

	UFUNCTION(BlueprintCallable, Category = "Quest System|Helper", meta = (WorldContext = "WorldContextObject"))
	static bool NotifyObjectiveProgressByTag(UObject* WorldContextObject, const FGameplayTag& ObjectiveTag, int32 Delta = 1);

	/** Broadcast GameplayMessageRouter event (handled by UExQuestMessageRouterBridge). */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Events", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "OptionalTaskId"))
	static void BroadcastQuestObjectiveProgress(UObject* WorldContextObject, const FGameplayTag& ObjectiveTag, int32 Delta = 1, FGameplayTag OptionalTaskId = FGameplayTag());

	UFUNCTION(BlueprintCallable, Category = "Quest System|DataAsset", meta = (WorldContext = "WorldContextObject"))
	static void LoadQuestFromAsset(UObject* WorldContextObject, UExQuestDataAsset* QuestAsset, bool bPreserveRuntime = false);

	UFUNCTION(BlueprintPure, Category = "Quest System|DataAsset")
	static FExQuestData BuildQuestDataFromAsset(const UExQuestDataAsset* QuestAsset);

	/** Build FExQuestData from a DataTable whose row struct is FExQuestTaskTableRow. */
	UFUNCTION(BlueprintCallable, Category = "Quest System|Create")
	static FExQuestData BuildQuestDataFromTaskTable(
		const UDataTable* TaskTable,
		const FText& InQuestSetName,
		const FString& InQuestSetId = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Quest System|Save", meta = (WorldContext = "WorldContextObject"))
	static FString SaveQuestProgressAsJson(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Quest System|Save", meta = (WorldContext = "WorldContextObject"))
	static bool LoadQuestProgressFromJson(UObject* WorldContextObject, const FString& JsonSaveData);

	UFUNCTION(BlueprintPure, Category = "Quest System|Runtime")
	static FExQuestRuntimeState ExtractRuntimeStateFromData(const FExQuestData& QuestData);

	UFUNCTION(BlueprintCallable, Category = "Quest System|Runtime", meta = (WorldContext = "WorldContextObject"))
	static void ApplyRuntimeStateToManager(UObject* WorldContextObject, const FExQuestRuntimeState& RuntimeState);

	UFUNCTION(BlueprintPure, Category = "Quest System|Helper")
	static FGameplayTag MakeQuestTag(const FString& TagString);

	UFUNCTION(BlueprintPure, Category = "Quest System|Helper")
	static bool HasSubTasks(const FExQuestTask& Task);

	UFUNCTION(BlueprintPure, Category = "Quest System|Helper")
	static bool HasPreTasks(const FExQuestTask& Task);

	UFUNCTION(BlueprintPure, Category = "Quest System|Helper")
	static int32 GetQuestCount(const FExQuestData& QuestData);
};
