// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/LatentTasks/ExLatentTask_Timer.h"

#include "Kismet/GameplayStatics.h"

UExLatentTask_Timer* UExLatentTask_Timer::CreateTimerProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_Timer> Class)
{
	if (!Class || !WorldContextObject)
	{
		return nullptr;
	}

	if (!Class->IsChildOf(UExLatentTask_Timer::StaticClass()))
	{
		return nullptr;
	}

	return Cast<UExLatentTask_Timer>(UGameplayStatics::SpawnObject(Class, WorldContextObject));
}

void UExLatentTask_Timer::OnStart()
{
	ResolvedDuration = 0.f;
	RemainingTime = 0.f;
	ElapsedTime = 0.f;
	bProgressRollingBack = false;

	Super::OnStart();

	const float Resolved = ResolveDuration();
	if (Resolved <= 0.f)
	{
		UE_LOG(LogLatentTask, Warning, TEXT("UExLatentTask_Timer: Duration must be > 0 for '%s'."), *GetName());
	}

	SetupCountdownTimer(Resolved);
	SyncMirrorProperties();

	if (bStartTimerOnStart)
	{
		CountdownTimer.StartCountdownInternal(true, false);
		SyncMirrorProperties();
	}
}

float UExLatentTask_Timer::ResolveDuration() const
{
	return Duration;
}

FExLatentCountdownTimerConfig UExLatentTask_Timer::BuildCountdownConfig(float InResolvedDuration) const
{
	FExLatentCountdownTimerConfig Config;
	Config.Duration = InResolvedDuration;
	Config.TickInterval = TickInterval;
	Config.bRollbackOnReset = bRollbackOnReset;
	Config.RollbackRate = RollbackRate;
	return Config;
}

void UExLatentTask_Timer::SetupCountdownTimer(float InResolvedDuration)
{
	const TWeakObjectPtr<UExLatentTask_Timer> WeakThis(this);

	FExLatentCountdownTimerCallbacks Callbacks;
	Callbacks.IsTaskRunning = [WeakThis]()
	{
		const UExLatentTask_Timer* Self = WeakThis.Get();
		return Self && Self->IsRunning();
	};
	Callbacks.OnTick = [WeakThis](float RemainingSeconds, float ElapsedSeconds)
	{
		UExLatentTask_Timer* Self = WeakThis.Get();
		if (!Self)
		{
			return;
		}

		Self->SyncMirrorProperties();
		Self->ReceiveOnTimerTick(RemainingSeconds, ElapsedSeconds);
	};
	Callbacks.OnNaturalComplete = [WeakThis]()
	{
		UExLatentTask_Timer* Self = WeakThis.Get();
		if (!Self)
		{
			return;
		}

		Self->SyncMirrorProperties();
		Self->TryStop();
	};
	Callbacks.OnElapsedReset = [WeakThis](bool /*bResetProgress*/)
	{
		if (UExLatentTask_Timer* Self = WeakThis.Get())
		{
			Self->SyncMirrorProperties();
		}
	};
	Callbacks.ShouldResetProgressOnRollbackComplete = []() { return false; };

	CountdownTimer.Initialize(GetWorld(), this, BuildCountdownConfig(InResolvedDuration), Callbacks);
	CountdownTimer.SetResolvedDuration(InResolvedDuration);
}

void UExLatentTask_Timer::SyncMirrorProperties()
{
	ResolvedDuration = CountdownTimer.GetResolvedDuration();
	RemainingTime = CountdownTimer.GetRemainingTime();
	ElapsedTime = CountdownTimer.GetElapsedTime();
	bProgressRollingBack = CountdownTimer.IsProgressRollingBack();
}

void UExLatentTask_Timer::StartCountdown()
{
	CountdownTimer.StartCountdown();
	SyncMirrorProperties();
}

void UExLatentTask_Timer::PauseCountdown()
{
	CountdownTimer.PauseCountdown();
	SyncMirrorProperties();
}

void UExLatentTask_Timer::ResumeCountdown()
{
	CountdownTimer.ResumeCountdown(false);
	SyncMirrorProperties();
}

void UExLatentTask_Timer::RestartCountdown()
{
	CountdownTimer.RestartCountdown(false);
	SyncMirrorProperties();
}

void UExLatentTask_Timer::ResetCountdown()
{
	CountdownTimer.ResetCountdown(false);
	SyncMirrorProperties();
}

void UExLatentTask_Timer::StartProgressRollback()
{
	CountdownTimer.StartProgressRollback();
	SyncMirrorProperties();
}

void UExLatentTask_Timer::InterruptTimer()
{
	CountdownTimer.InterruptTimer();
	SyncMirrorProperties();
}

void UExLatentTask_Timer::CancelTimer()
{
	if (!IsRunning() && !IsPending())
	{
		return;
	}

	CountdownTimer.InterruptTimer();
	SyncMirrorProperties();
}

bool UExLatentTask_Timer::IsCountdownPaused() const
{
	return CountdownTimer.IsCountdownPaused();
}

bool UExLatentTask_Timer::IsProgressRollbackPaused() const
{
	return CountdownTimer.IsProgressRollbackPaused();
}

void UExLatentTask_Timer::OnStop()
{
	CountdownTimer.Shutdown();
	Super::OnStop();
}

void UExLatentTask_Timer::BeginDestroy()
{
	CountdownTimer.Shutdown();
	Super::BeginDestroy();
}
