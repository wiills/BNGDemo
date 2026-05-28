// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/LatentTasks/ExLatentCountdownTimer.h"

void FExLatentCountdownTimer::Initialize(
	UWorld* InWorld,
	UObject* InOwner,
	const FExLatentCountdownTimerConfig& InConfig,
	const FExLatentCountdownTimerCallbacks& InCallbacks)
{
	Shutdown();
	World = InWorld;
	OwnerWeak = InOwner;
	Config = InConfig;
	Callbacks = InCallbacks;
	ResolvedDuration = FMath::Max(0.f, Config.Duration);
	RemainingTime = ResolvedDuration;
	ElapsedTime = 0.f;
	bCountdownActive = false;
	bProgressRollingBack = false;
}

void FExLatentCountdownTimer::Shutdown()
{
	ClearCountdownTimer();
	World = nullptr;
	OwnerWeak.Reset();
	Callbacks = FExLatentCountdownTimerCallbacks();
}

void FExLatentCountdownTimer::SetResolvedDuration(float InDuration)
{
	ResolvedDuration = FMath::Max(0.f, InDuration);
	RemainingTime = FMath::Max(0.f, ResolvedDuration - ElapsedTime);
}

void FExLatentCountdownTimer::StartCountdown()
{
	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	if (bCountdownActive && !bProgressRollingBack)
	{
		return;
	}

	ClearCountdownTimer();
	bProgressRollingBack = false;
	StartCountdownInternal(ElapsedTime <= 0.f, Config.bResetProgressOnStart);
}

void FExLatentCountdownTimer::PauseCountdown()
{
	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	if (!bCountdownActive)
	{
		return;
	}

	ClearCountdownTimer();
}

void FExLatentCountdownTimer::ResumeCountdown(bool bResetProgressOnReset)
{
	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	if (bCountdownActive)
	{
		return;
	}

	if (bProgressRollingBack)
	{
		if (ElapsedTime <= 0.f)
		{
			FinishProgressRollback();
			return;
		}

		BeginCountdownTimer();
		return;
	}

	if (ElapsedTime >= ResolvedDuration)
	{
		return;
	}

	BeginCountdownTimer();
}

void FExLatentCountdownTimer::RestartCountdown(bool bResetProgressOnStart)
{
	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	bProgressRollingBack = false;
	ClearCountdownTimer();
	StartCountdownInternal(true, bResetProgressOnStart);
}

void FExLatentCountdownTimer::ResetCountdown(bool bResetProgressOnReset)
{
	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	if (Config.bRollbackOnReset && ElapsedTime > 0.f)
	{
		StartProgressRollbackInternal();
		return;
	}

	ResetCountdownElapsed(bResetProgressOnReset);
}

void FExLatentCountdownTimer::StartProgressRollback()
{
	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	StartProgressRollbackInternal();
}

void FExLatentCountdownTimer::InterruptTimer()
{
	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	ClearCountdownTimer();
	bProgressRollingBack = false;
}

bool FExLatentCountdownTimer::IsCountdownPaused() const
{
	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return false;
	}

	return !bCountdownActive
		&& !bProgressRollingBack
		&& ElapsedTime > 0.f
		&& ElapsedTime < ResolvedDuration;
}

bool FExLatentCountdownTimer::IsProgressRollbackPaused() const
{
	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return false;
	}

	return bProgressRollingBack && !bCountdownActive && ElapsedTime > 0.f;
}

void FExLatentCountdownTimer::StartCountdownInternal(bool bResetElapsed, bool bResetProgressOnStart)
{
	if (bCountdownActive)
	{
		return;
	}

	bProgressRollingBack = false;

	if (bResetElapsed)
	{
		ResetCountdownElapsed(bResetProgressOnStart);
	}

	if (ResolvedDuration <= 0.f)
	{
		CompleteCountdown();
		return;
	}

	if (ElapsedTime >= ResolvedDuration)
	{
		CompleteCountdown();
		return;
	}

	BeginCountdownTimer();
}

void FExLatentCountdownTimer::ResetCountdownElapsed(bool bResetProgress)
{
	ClearCountdownTimer();
	bProgressRollingBack = false;
	ElapsedTime = 0.f;
	RemainingTime = ResolvedDuration;

	if (bResetProgress && Callbacks.OnElapsedReset)
	{
		Callbacks.OnElapsedReset(true);
	}
}

void FExLatentCountdownTimer::StartProgressRollbackInternal()
{
	if (ElapsedTime <= 0.f)
	{
		return;
	}

	ClearCountdownTimer();
	bProgressRollingBack = true;
	BeginCountdownTimer();
}

void FExLatentCountdownTimer::StopProgressRollback(bool bResetProgress)
{
	ClearCountdownTimer();
	bProgressRollingBack = false;
	ElapsedTime = 0.f;
	RemainingTime = ResolvedDuration;

	if (bResetProgress && Callbacks.OnElapsedReset)
	{
		Callbacks.OnElapsedReset(true);
	}
}

void FExLatentCountdownTimer::FinishProgressRollback()
{
	const bool bResetProgress = Callbacks.ShouldResetProgressOnRollbackComplete
		? Callbacks.ShouldResetProgressOnRollbackComplete()
		: false;
	StopProgressRollback(bResetProgress);
}

void FExLatentCountdownTimer::BeginCountdownTimer()
{
	if (bCountdownActive)
	{
		return;
	}

	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	if (bProgressRollingBack)
	{
		if (ElapsedTime <= 0.f)
		{
			FinishProgressRollback();
			return;
		}
	}
	else if (ElapsedTime >= ResolvedDuration)
	{
		CompleteCountdown();
		return;
	}

	if (!World || !OwnerWeak.IsValid())
	{
		return;
	}

	RemainingTime = FMath::Max(0.f, ResolvedDuration - ElapsedTime);
	bCountdownActive = true;

	const float FirstDelay = bProgressRollingBack
		? Config.TickInterval
		: FMath::Min(Config.TickInterval, RemainingTime);

	const TWeakObjectPtr<UObject> WeakOwner = OwnerWeak;
	const TWeakObjectPtr<UWorld> WeakWorld = World;

	WeakWorld->GetTimerManager().SetTimer(
		CountdownTickHandle,
		[WeakOwner, this]()
		{
			if (!WeakOwner.IsValid())
			{
				return;
			}

			if (bProgressRollingBack)
			{
				HandleProgressRollbackTick();
			}
			else
			{
				HandleTimerTick();
			}
		},
		Config.TickInterval,
		true,
		FirstDelay);
}

void FExLatentCountdownTimer::ClearCountdownTimer()
{
	bCountdownActive = false;

	if (CountdownTickHandle.IsValid())
	{
		if (World)
		{
			World->GetTimerManager().ClearTimer(CountdownTickHandle);
		}
		CountdownTickHandle.Invalidate();
	}
}

void FExLatentCountdownTimer::HandleTimerTick()
{
	if (!OwnerWeak.IsValid())
	{
		ClearCountdownTimer();
		return;
	}

	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	if (bProgressRollingBack)
	{
		return;
	}

	ElapsedTime = FMath::Min(ElapsedTime + Config.TickInterval, ResolvedDuration);
	RemainingTime = FMath::Max(0.f, ResolvedDuration - ElapsedTime);

	if (Callbacks.OnTick)
	{
		Callbacks.OnTick(RemainingTime, ElapsedTime);
	}

	if (ElapsedTime >= ResolvedDuration)
	{
		CompleteCountdown();
	}
}

void FExLatentCountdownTimer::HandleProgressRollbackTick()
{
	if (!OwnerWeak.IsValid())
	{
		ClearCountdownTimer();
		return;
	}

	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	if (!bProgressRollingBack)
	{
		return;
	}

	const float RollbackDelta = Config.RollbackRate * Config.TickInterval;
	ElapsedTime = FMath::Max(0.f, ElapsedTime - RollbackDelta);
	RemainingTime = FMath::Max(0.f, ResolvedDuration - ElapsedTime);

	if (Callbacks.OnTick)
	{
		Callbacks.OnTick(RemainingTime, ElapsedTime);
	}

	if (ElapsedTime <= 0.f)
	{
		FinishProgressRollback();
	}
}

void FExLatentCountdownTimer::CompleteCountdown()
{
	if (!OwnerWeak.IsValid())
	{
		ClearCountdownTimer();
		return;
	}

	if (Callbacks.IsTaskRunning && !Callbacks.IsTaskRunning())
	{
		return;
	}

	ClearCountdownTimer();
	RemainingTime = 0.f;
	ElapsedTime = ResolvedDuration;

	if (Callbacks.OnNaturalComplete)
	{
		Callbacks.OnNaturalComplete();
	}
}
