// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/LatentTasks/ExLatentTask_Quest.h"
#include "ExLatentTask_QuestTimer.generated.h"

/**
 * Quest latent task with a countdown timer.
 * When ObjectiveTag is set, elapsed seconds sync to CurrentProgress (set DT TargetProgress = Duration).
 * Fires ReceiveOnTimerTick every TickInterval (default 1s); completes when Duration elapses.
 */
UCLASS(Blueprintable, BlueprintType, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen, DontUseGenericSpawnObject))
class BLUEPRINTNODEGRAPH_API UExLatentTask_QuestTimer : public UExLatentTask_Quest
{
	GENERATED_BODY()

public:
	/** Total countdown duration in seconds; DT Objective TargetProgress should match (e.g. Duration=10 -> TargetProgress=10) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ExposeOnSpawn = true, ClampMin = "0.0"))
	float Duration = 10.0f;

	/** Interval between ReceiveOnTimerTick events in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ExposeOnSpawn = true, ClampMin = "0.01"))
	float TickInterval = 1.0f;

	/** Sync elapsed seconds to Objective CurrentProgress while running */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ExposeOnSpawn = true))
	bool bSyncObjectiveProgress = true;

	/** Reset Objective progress to 0 when the countdown starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ExposeOnSpawn = true, EditCondition = "bSyncObjectiveProgress"))
	bool bResetProgressOnStart = true;

	/** Reset Objective progress to 0 when CancelTimer is called */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ExposeOnSpawn = true, EditCondition = "bSyncObjectiveProgress"))
	bool bResetProgressOnCancel = true;

	/** Seconds remaining in the countdown */
	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	float RemainingTime = 0.0f;

	/** Seconds elapsed since the countdown started */
	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	float ElapsedTime = 0.0f;

	/** Last synced Objective CurrentProgress value */
	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	int32 SyncedObjectiveProgress = 0;

	UFUNCTION(BlueprintCallable, Category = "Quest|Timer")
	void InterruptTimer();

	UFUNCTION(BlueprintCallable, Category = "Quest|Timer")
	void CancelTimer();

	UFUNCTION(BlueprintPure, Category = "Quest|Timer")
	float GetRemainingTime() const { return RemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Quest|Timer")
	float GetElapsedTime() const { return ElapsedTime; }

	UFUNCTION(BlueprintPure, Category = "Quest|Timer")
	int32 GetSyncedObjectiveProgress() const { return SyncedObjectiveProgress; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest|Timer", meta = (DisplayName = "On Timer Tick"))
	void ReceiveOnTimerTick(float RemainingSeconds, float ElapsedSeconds);

protected:
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual void BeginDestroy() override;
	virtual void ApplyQuestOnComplete_Implementation() override;

	UFUNCTION()
	void HandleTimerTick();

	void ClearCountdownTimer();
	void CompleteCountdown();
	void SyncObjectiveProgressFromElapsed();
	void ResetObjectiveProgress();
	int32 ComputeObjectiveProgressFromElapsed() const;

	UPROPERTY()
	FTimerHandle CountdownTickHandle;

	bool bCompletedNaturally = false;
};
