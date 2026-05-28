// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/LatentTasks/ExLatentCountdownTimer.h"
#include "BlueprintTool/LatentTasks/ExLatentTask_Custom.h"
#include "ExLatentTask_Timer.generated.h"

/**
 * Created via UExK2Node_TimerTask / CreateTimerProxy (not Create Latent Task).
 * Duration is configured on the Timer Task node via ExposeOnSpawn.
 * Enter/leave volume: bStartTimerOnStart=false, enter->StartCountdown, leave->ResetCountdown (or StartProgressRollback).
 */
UCLASS(Blueprintable, BlueprintType, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen, DontUseGenericSpawnObject))
class BLUEPRINTNODEGRAPH_API UExLatentTask_Timer : public UExLatentTask_Custom
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "LatentTasks|Timer", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Create Timer Latent Task"))
	static UExLatentTask_Timer* CreateTimerProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_Timer> Class);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ExposeOnSpawn = true, ClampMin = "0.0"))
	float Duration = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timer", meta = (ClampMin = "0.01"))
	float TickInterval = 1.f;

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

	UPROPERTY(BlueprintReadOnly, Category = "Timer|Rollback")
	bool bProgressRollingBack = false;

	UFUNCTION(BlueprintCallable, Category = "LatentTask|Timer")
	void StartCountdown();

	UFUNCTION(BlueprintCallable, Category = "LatentTask|Timer")
	void PauseCountdown();

	UFUNCTION(BlueprintCallable, Category = "LatentTask|Timer")
	void ResumeCountdown();

	UFUNCTION(BlueprintCallable, Category = "LatentTask|Timer")
	void RestartCountdown();

	UFUNCTION(BlueprintCallable, Category = "LatentTask|Timer")
	void ResetCountdown();

	UFUNCTION(BlueprintCallable, Category = "LatentTask|Timer|Rollback")
	void StartProgressRollback();

	UFUNCTION(BlueprintCallable, Category = "LatentTask|Timer")
	void InterruptTimer();

	UFUNCTION(BlueprintCallable, Category = "LatentTask|Timer")
	void CancelTimer();

	UFUNCTION(BlueprintPure, Category = "LatentTask|Timer")
	float GetResolvedDuration() const { return ResolvedDuration; }

	UFUNCTION(BlueprintPure, Category = "LatentTask|Timer")
	float GetRemainingTime() const { return RemainingTime; }

	UFUNCTION(BlueprintPure, Category = "LatentTask|Timer")
	float GetElapsedTime() const { return ElapsedTime; }

	UFUNCTION(BlueprintPure, Category = "LatentTask|Timer")
	bool IsCountdownActive() const { return CountdownTimer.IsCountdownActive(); }

	UFUNCTION(BlueprintPure, Category = "LatentTask|Timer")
	bool IsCountdownPaused() const;

	UFUNCTION(BlueprintPure, Category = "LatentTask|Timer|Rollback")
	bool IsProgressRollingBack() const { return bProgressRollingBack; }

	UFUNCTION(BlueprintPure, Category = "LatentTask|Timer|Rollback")
	bool IsProgressRollbackPaused() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "LatentTask|Timer", meta = (DisplayName = "On Timer Tick"))
	void ReceiveOnTimerTick(float RemainingSeconds, float ElapsedSeconds);

protected:
	virtual void OnStart() override;
	virtual void OnStop() override;
	virtual void BeginDestroy() override;

	virtual float ResolveDuration() const;
	virtual FExLatentCountdownTimerConfig BuildCountdownConfig(float InResolvedDuration) const;
	virtual void SetupCountdownTimer(float InResolvedDuration);
	void SyncMirrorProperties();

	FExLatentCountdownTimer CountdownTimer;
};
