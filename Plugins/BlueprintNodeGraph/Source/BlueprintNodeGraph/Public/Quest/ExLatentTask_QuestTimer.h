// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/LatentTasks/ExLatentTask_Quest.h"
#include "ExLatentTask_QuestTimer.generated.h"

/**
 * Quest latent task with a countdown timer.
 * Duration on Quest Task node: >0 manual seconds, 0 = read Objective TargetProgress from quest data.
 * Enter/leave volume (reset on leave): bStartTimerOnStart=false, enter->RestartCountdown, leave->ResetCountdown.
 */
UCLASS(Blueprintable, BlueprintType, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen, DontUseGenericSpawnObject))
class BLUEPRINTNODEGRAPH_API UExLatentTask_QuestTimer : public UExLatentTask_Quest
{
	GENERATED_BODY()

public:
	/**
	 * Countdown length override (seconds). Exposed on Quest Task node only.
	 * <= 0: use Objective TargetProgress from loaded quest data (requires QuestTag + ObjectiveTag).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ExposeOnSpawn = true, ClampMin = "0.0", AdvancedDisplay))
	float Duration = 0.0f;

	/** Interval between ReceiveOnTimerTick events in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ClampMin = "0.01"))
	float TickInterval = 1.0f;

	/** Sync elapsed seconds to Objective CurrentProgress while running */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	bool bSyncObjectiveProgress = true;

	/** Reset Objective progress to 0 when the countdown starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (EditCondition = "bSyncObjectiveProgress"))
	bool bResetProgressOnStart = true;

	/** Reset Objective progress to 0 when CancelTimer is called */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (EditCondition = "bSyncObjectiveProgress"))
	bool bResetProgressOnCancel = true;

	/** Reset Objective progress to 0 when ResetCountdown is called */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (EditCondition = "bSyncObjectiveProgress"))
	bool bResetProgressOnReset = true;

	/** When true the countdown starts in OnStart; when false call StartCountdown manually */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	bool bStartTimerOnStart = true;

	/** Resolved countdown length after OnStart (manual Duration or Objective TargetProgress) */
	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	float ResolvedDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	float RemainingTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	float ElapsedTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	int32 SyncedObjectiveProgress = 0;

	UFUNCTION(BlueprintCallable, Category = "Quest|Timer")
	void StartCountdown();

	UFUNCTION(BlueprintCallable, Category = "Quest|Timer")
	void PauseCountdown();

	UFUNCTION(BlueprintCallable, Category = "Quest|Timer")
	void ResumeCountdown();

	UFUNCTION(BlueprintCallable, Category = "Quest|Timer")
	void RestartCountdown();

	UFUNCTION(BlueprintCallable, Category = "Quest|Timer")
	void ResetCountdown();

	UFUNCTION(BlueprintCallable, Category = "Quest|Timer")
	void InterruptTimer();

	UFUNCTION(BlueprintCallable, Category = "Quest|Timer")
	void CancelTimer();

	UFUNCTION(BlueprintPure, Category = "Quest|Timer")
	float GetResolvedDuration() const { return ResolvedDuration; }

	UFUNCTION(BlueprintPure, Category = "Quest|Timer")
	float GetRemainingTime() const { return RemainingTime; }

	UFUNCTION(BlueprintPure, Category = "Quest|Timer")
	float GetElapsedTime() const { return ElapsedTime; }

	UFUNCTION(BlueprintPure, Category = "Quest|Timer")
	int32 GetSyncedObjectiveProgress() const { return SyncedObjectiveProgress; }

	UFUNCTION(BlueprintPure, Category = "Quest|Timer")
	bool IsCountdownActive() const { return bCountdownActive; }

	UFUNCTION(BlueprintPure, Category = "Quest|Timer")
	bool IsCountdownPaused() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest|Timer", meta = (DisplayName = "On Timer Tick"))
	void ReceiveOnTimerTick(float RemainingSeconds, float ElapsedSeconds);

protected:
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual void BeginDestroy() override;
	virtual void ApplyQuestOnComplete_Implementation() override;

	UFUNCTION()
	void HandleTimerTick();

	bool ResolveDuration();
	bool TryGetObjectiveTargetProgress(int32& OutTargetProgress) const;

	void ClearCountdownTimer();
	void BeginCountdownTimer();
	void StartCountdownInternal(bool bResetElapsed);
	void ResetCountdownElapsed(bool bResetObjectiveProgress);
	void CompleteCountdown();
	void SyncObjectiveProgressFromElapsed();
	void ResetObjectiveProgress();
	int32 ComputeObjectiveProgressFromElapsed() const;

	UPROPERTY()
	FTimerHandle CountdownTickHandle;

	bool bCompletedNaturally = false;
	bool bCountdownActive = false;
};
