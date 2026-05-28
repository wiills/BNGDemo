// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/LatentTasks/ExLatentCountdownTimer.h"
#include "BlueprintTool/LatentTasks/ExLatentTask_Quest.h"
#include "ExLatentTask_QuestTimer.generated.h"

/**
 * Quest latent task with a countdown timer.
 * Countdown length is read from Objective TargetProgress in loaded quest data (QuestTag + ObjectiveTag).
 * Enter/leave volume: bStartTimerOnStart=false, enter->StartCountdown, leave->ResetCountdown (or StartProgressRollback).
 * With bRollbackOnReset, leave gradually rolls ElapsedTime back at RollbackRate points/sec; re-enter resumes from current progress.
 */
UCLASS(Blueprintable, BlueprintType, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen, DontUseGenericSpawnObject))
class BLUEPRINTNODEGRAPH_API UExLatentTask_QuestTimer : public UExLatentTask_Quest
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ClampMin = "0.01"))
	float TickInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	bool bSyncObjectiveProgress = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (EditCondition = "bSyncObjectiveProgress"))
	bool bResetProgressOnStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (EditCondition = "bSyncObjectiveProgress"))
	bool bResetProgressOnCancel = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (EditCondition = "bSyncObjectiveProgress"))
	bool bResetProgressOnReset = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer")
	bool bStartTimerOnStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer|Rollback")
	bool bRollbackOnReset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer|Rollback", meta = (ClampMin = "0.01"))
	float RollbackRate = 3.f;

	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	float ResolvedDuration = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	float RemainingTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	float ElapsedTime = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Timer")
	int32 SyncedObjectiveProgress = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Timer|Rollback")
	bool bProgressRollingBack = false;

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

	UFUNCTION(BlueprintCallable, Category = "Quest|Timer|Rollback")
	void StartProgressRollback();

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
	bool IsCountdownActive() const { return CountdownTimer.IsCountdownActive(); }

	UFUNCTION(BlueprintPure, Category = "Quest|Timer")
	bool IsCountdownPaused() const;

	UFUNCTION(BlueprintPure, Category = "Quest|Timer|Rollback")
	bool IsProgressRollingBack() const { return bProgressRollingBack; }

	UFUNCTION(BlueprintPure, Category = "Quest|Timer|Rollback")
	bool IsProgressRollbackPaused() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest|Timer", meta = (DisplayName = "On Timer Tick"))
	void ReceiveOnTimerTick(float RemainingSeconds, float ElapsedSeconds);

protected:
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual void BeginDestroy() override;
	virtual void ApplyQuestOnComplete_Implementation() override;

	bool ResolveDuration();
	bool TryGetObjectiveTargetProgress(int32& OutTargetProgress) const;
	FExLatentCountdownTimerConfig BuildCountdownConfig(float InResolvedDuration) const;
	void SetupCountdownTimer(float InResolvedDuration);
	void SyncMirrorProperties();

	void SyncObjectiveProgressFromElapsed();
	void ResetObjectiveProgress();
	int32 ComputeObjectiveProgressFromElapsed() const;

	FExLatentCountdownTimer CountdownTimer;
	bool bCompletedNaturally = false;
};
