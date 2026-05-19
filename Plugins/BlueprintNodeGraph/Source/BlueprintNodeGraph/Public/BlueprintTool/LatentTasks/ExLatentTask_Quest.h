// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BlueprintTool/LatentTasks/ExBase_LatentTask.h"
#include "ExLatentTask_Quest.generated.h"

UENUM(BlueprintType)
enum class EExQuestCompleteAction : uint8
{
	IncrementProgress UMETA(DisplayName = "Increment Progress"),
	CompleteObjective UMETA(DisplayName = "Complete Objective")
};

/**
 * Base class for all quest-related latent tasks.
 * Blueprint subclasses implement gameplay; on successful TryStop the task can push progress to the quest system.
 * Created via UExK2Node_QuestTask / CreateQuestProxy (not CreateLatentTask).
 */
UCLASS(Blueprintable, BlueprintType, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen, DontUseGenericSpawnObject))
class BLUEPRINTNODEGRAPH_API UExLatentTask_Quest : public UExBase_LatentTask
{
	GENERATED_BODY()

public:
	/** Quest configuration tag (TaskId), not a world instance id */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest", ExposeOnSpawn = true))
	FGameplayTag BoundQuestTag;

	/** Objective configuration tag, not a world instance id */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest", ExposeOnSpawn = true))
	FGameplayTag BoundObjectiveTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ExposeOnSpawn = true))
	EExQuestCompleteAction CompleteAction = EExQuestCompleteAction::IncrementProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ExposeOnSpawn = true, ClampMin = "1"))
	int32 ProgressDeltaOnStop = 1;

	/** When true and state is Completed, ApplyQuestOnComplete runs in OnStop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ExposeOnSpawn = true))
	bool bApplyQuestOnSuccessfulStop = true;

	UFUNCTION(BlueprintCallable, Category = "LatentTasks|Quest", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Create Quest Latent Task"))
	static UExLatentTask_Quest* CreateQuestProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_Quest> Class);

	UFUNCTION(BlueprintPure, Category = "LatentTasks|Quest")
	static bool IsQuestLatentClass(TSubclassOf<UExLatentTask_Quest> Class);

protected:
	virtual void OnStop() override;

	/** Override to customize how quest state is updated (default: increment or complete objective) */
	UFUNCTION(BlueprintNativeEvent, Category = "Quest")
	void ApplyQuestOnComplete();
	virtual void ApplyQuestOnComplete_Implementation();
};
