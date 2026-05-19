// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "BlueprintTool/LatentTasks/ExBase_LatentTask.h"
#include "ExLatentTask_QuestBound.generated.h"

UENUM(BlueprintType)
enum class EExQuestBoundCompleteAction : uint8
{
	IncrementProgress UMETA(DisplayName = "Increment Progress"),
	CompleteObjective UMETA(DisplayName = "Complete Objective")
};

/**
 * Latent task bound to quest objectives.
 * Created via UExK2Node_QuestTask / CreateQuestBoundProxy (not CreateLatentTask).
 */
UCLASS(Blueprintable, BlueprintType, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen, DontUseGenericSpawnObject))
class BLUEPRINTNODEGRAPH_API UExLatentTask_QuestBound : public UExBase_LatentTask
{
	GENERATED_BODY()

public:
	/** Quest configuration tag (not instance id) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest", ExposeOnSpawn = true))
	FGameplayTag BoundQuestTag;

	/** Objective configuration tag (not instance id) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (Categories = "Quest", ExposeOnSpawn = true))
	FGameplayTag BoundObjectiveTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ExposeOnSpawn = true))
	EExQuestBoundCompleteAction CompleteAction = EExQuestBoundCompleteAction::IncrementProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ExposeOnSpawn = true, ClampMin = "1"))
	int32 ProgressDeltaOnStop = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest", meta = (ExposeOnSpawn = true))
	bool bApplyQuestOnSuccessfulStop = true;

	UFUNCTION(BlueprintCallable, Category = "LatentTasks|Quest", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Create Quest Bound Latent Task"))
	static UExLatentTask_QuestBound* CreateQuestBoundProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_QuestBound> Class);

	UFUNCTION(BlueprintPure, Category = "LatentTasks|Quest")
	static bool IsQuestBoundLatentClass(TSubclassOf<UExLatentTask_QuestBound> Class);

protected:
	virtual void OnStop() override;

	void ApplyQuestBindingOnComplete();
};
