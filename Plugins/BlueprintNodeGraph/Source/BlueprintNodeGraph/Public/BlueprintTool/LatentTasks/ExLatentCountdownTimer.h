// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"

/** Static configuration for a countdown timer instance */
struct FExLatentCountdownTimerConfig
{
	float Duration = 0.f;
	float TickInterval = 1.f;
	bool bRollbackOnReset = false;
	float RollbackRate = 3.f;
	bool bResetProgressOnStart = false;
};

/** Owner-provided hooks; keep UObject-specific logic out of the core timer */
struct FExLatentCountdownTimerCallbacks
{
	TFunction<bool()> IsTaskRunning;
	TFunction<void(float RemainingSeconds, float ElapsedSeconds)> OnTick;
	TFunction<void()> OnNaturalComplete;
	TFunction<void(bool bResetProgress)> OnElapsedReset;
	/** Used when rollback reaches zero (ResetCountdown / ResumeCountdown paths) */
	TFunction<bool()> ShouldResetProgressOnRollbackComplete;
};

/**
 * Reusable countdown state machine (tick, pause, resume, rollback).
 * Used by UExLatentTask_Timer and UExLatentTask_QuestTimer.
 */
class BLUEPRINTNODEGRAPH_API FExLatentCountdownTimer
{
public:
	void Initialize(UWorld* InWorld, UObject* InOwner, const FExLatentCountdownTimerConfig& InConfig, const FExLatentCountdownTimerCallbacks& InCallbacks);
	void Shutdown();

	void SetResolvedDuration(float InDuration);

	void StartCountdown();
	void PauseCountdown();
	void ResumeCountdown(bool bResetProgressOnReset);
	void RestartCountdown(bool bResetProgressOnStart);
	void ResetCountdown(bool bResetProgressOnReset);
	void StartProgressRollback();
	void InterruptTimer();

	void StartCountdownInternal(bool bResetElapsed, bool bResetProgressOnStart);

	float GetResolvedDuration() const { return ResolvedDuration; }
	float GetRemainingTime() const { return RemainingTime; }
	float GetElapsedTime() const { return ElapsedTime; }
	bool IsCountdownActive() const { return bCountdownActive; }
	bool IsProgressRollingBack() const { return bProgressRollingBack; }

	bool IsCountdownPaused() const;
	bool IsProgressRollbackPaused() const;

private:
	void ClearCountdownTimer();
	void BeginCountdownTimer();
	void ResetCountdownElapsed(bool bResetProgress);
	void StartProgressRollbackInternal();
	void StopProgressRollback(bool bResetProgress);
	void FinishProgressRollback();
	void HandleTimerTick();
	void HandleProgressRollbackTick();
	void CompleteCountdown();

	UWorld* World = nullptr;
	TWeakObjectPtr<UObject> OwnerWeak;
	FExLatentCountdownTimerConfig Config;
	FExLatentCountdownTimerCallbacks Callbacks;

	FTimerHandle CountdownTickHandle;

	float ResolvedDuration = 0.f;
	float RemainingTime = 0.f;
	float ElapsedTime = 0.f;

	bool bCountdownActive = false;
	bool bProgressRollingBack = false;
};
