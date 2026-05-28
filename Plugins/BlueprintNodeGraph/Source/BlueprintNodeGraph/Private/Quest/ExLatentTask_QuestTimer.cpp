// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExLatentTask_QuestTimer.h"

#include "Quest/ExQuestBlueprintLibrary.h"
#include "Quest/ExQuestManagerSubsystem.h"
#include "Quest/ExQuestReplicationComponent.h"

void UExLatentTask_QuestTimer::OnStart()
{
	SyncedObjectiveProgress = 0;
	bCompletedNaturally = false;
	ResolvedDuration = 0.f;
	RemainingTime = 0.f;
	ElapsedTime = 0.f;
	bProgressRollingBack = false;

	Super::OnStart();

	if (!ResolveDuration())
	{
		UE_LOG(LogLatentTask, Warning, TEXT("UExLatentTask_QuestTimer: failed to resolve duration for task '%s' objective '%s'."),
			*QuestTag.ToString(), *ObjectiveTag.ToString());
	}

	SetupCountdownTimer(ResolvedDuration);
	SyncMirrorProperties();

	if (bStartTimerOnStart)
	{
		CountdownTimer.StartCountdownInternal(true, bResetProgressOnStart);
		SyncMirrorProperties();
	}
}

bool UExLatentTask_QuestTimer::TryGetObjectiveTargetProgress(int32& OutTargetProgress) const
{
	OutTargetProgress = 0;

	if (!ObjectiveTag.IsValid() || !QuestTag.IsValid())
	{
		return false;
	}

	const UExQuestManagerSubsystem* Manager = UExQuestBlueprintLibrary::GetQuestManager(GetWorld());
	if (!Manager)
	{
		return false;
	}

	FExQuestTask Task;
	if (!Manager->GetQuestById(QuestTag, Task))
	{
		return false;
	}

	for (const FExQuestObjective& Objective : Task.Objectives)
	{
		if (Objective.ObjectiveTag == ObjectiveTag)
		{
			OutTargetProgress = FMath::Max(0, Objective.TargetProgress);
			return true;
		}
	}

	return false;
}

bool UExLatentTask_QuestTimer::ResolveDuration()
{
	int32 TargetProgress = 0;
	if (TryGetObjectiveTargetProgress(TargetProgress))
	{
		ResolvedDuration = static_cast<float>(TargetProgress);
		return ResolvedDuration > 0.f;
	}

	return false;
}

FExLatentCountdownTimerConfig UExLatentTask_QuestTimer::BuildCountdownConfig(float InResolvedDuration) const
{
	FExLatentCountdownTimerConfig Config;
	Config.Duration = InResolvedDuration;
	Config.TickInterval = TickInterval;
	Config.bRollbackOnReset = bRollbackOnReset;
	Config.RollbackRate = RollbackRate;
	Config.bResetProgressOnStart = bResetProgressOnStart;
	return Config;
}

void UExLatentTask_QuestTimer::SetupCountdownTimer(float InResolvedDuration)
{
	const TWeakObjectPtr<UExLatentTask_QuestTimer> WeakThis(this);

	FExLatentCountdownTimerCallbacks Callbacks;
	Callbacks.IsTaskRunning = [WeakThis]()
	{
		const UExLatentTask_QuestTimer* Self = WeakThis.Get();
		return Self && Self->IsRunning();
	};
	Callbacks.OnTick = [WeakThis](float RemainingSeconds, float ElapsedSeconds)
	{
		UExLatentTask_QuestTimer* Self = WeakThis.Get();
		if (!Self)
		{
			return;
		}

		Self->SyncMirrorProperties();
		Self->SyncObjectiveProgressFromElapsed();
		Self->ReceiveOnTimerTick(RemainingSeconds, ElapsedSeconds);
	};
	Callbacks.OnNaturalComplete = [WeakThis]()
	{
		UExLatentTask_QuestTimer* Self = WeakThis.Get();
		if (!Self)
		{
			return;
		}

		Self->bCompletedNaturally = true;
		Self->SyncMirrorProperties();
		// Final tick already synced progress; sync again for zero-duration paths, then stop.
		Self->SyncObjectiveProgressFromElapsed();
		Self->TryStop();
	};
	Callbacks.OnElapsedReset = [WeakThis](bool bResetProgress)
	{
		UExLatentTask_QuestTimer* Self = WeakThis.Get();
		if (!Self)
		{
			return;
		}

		Self->SyncMirrorProperties();
		if (Self->bSyncObjectiveProgress && bResetProgress && Self->ObjectiveTag.IsValid())
		{
			Self->ResetObjectiveProgress();
		}
	};
	Callbacks.ShouldResetProgressOnRollbackComplete = [WeakThis]()
	{
		const UExLatentTask_QuestTimer* Self = WeakThis.Get();
		return Self ? Self->bResetProgressOnReset : false;
	};

	CountdownTimer.Initialize(GetWorld(), this, BuildCountdownConfig(InResolvedDuration), Callbacks);
	CountdownTimer.SetResolvedDuration(InResolvedDuration);
}

void UExLatentTask_QuestTimer::SyncMirrorProperties()
{
	ResolvedDuration = CountdownTimer.GetResolvedDuration();
	RemainingTime = CountdownTimer.GetRemainingTime();
	ElapsedTime = CountdownTimer.GetElapsedTime();
	bProgressRollingBack = CountdownTimer.IsProgressRollingBack();
}

void UExLatentTask_QuestTimer::StartCountdown()
{
	CountdownTimer.StartCountdown();
	SyncMirrorProperties();
}

void UExLatentTask_QuestTimer::PauseCountdown()
{
	CountdownTimer.PauseCountdown();
	SyncMirrorProperties();
}

void UExLatentTask_QuestTimer::ResumeCountdown()
{
	CountdownTimer.ResumeCountdown(bResetProgressOnReset);
	SyncMirrorProperties();
}

void UExLatentTask_QuestTimer::RestartCountdown()
{
	CountdownTimer.RestartCountdown(bResetProgressOnStart);
	SyncMirrorProperties();
}

void UExLatentTask_QuestTimer::ResetCountdown()
{
	if (IsStopped() || IsCancelled())
	{
		return;
	}

	if (!IsRunning())
	{
		return;
	}

	CountdownTimer.ResetCountdown(bResetProgressOnReset);
	SyncMirrorProperties();
}

void UExLatentTask_QuestTimer::StartProgressRollback()
{
	CountdownTimer.StartProgressRollback();
	SyncMirrorProperties();
}

void UExLatentTask_QuestTimer::InterruptTimer()
{
	CountdownTimer.InterruptTimer();
	SyncMirrorProperties();
	SyncObjectiveProgressFromElapsed();
}

void UExLatentTask_QuestTimer::CancelTimer()
{
	if (!IsRunning() && !IsPending())
	{
		return;
	}

	CountdownTimer.InterruptTimer();
	SyncMirrorProperties();

	if (bSyncObjectiveProgress && bResetProgressOnCancel && ObjectiveTag.IsValid())
	{
		ResetObjectiveProgress();
	}
}

bool UExLatentTask_QuestTimer::IsCountdownPaused() const
{
	return CountdownTimer.IsCountdownPaused();
}

bool UExLatentTask_QuestTimer::IsProgressRollbackPaused() const
{
	return CountdownTimer.IsProgressRollbackPaused();
}

void UExLatentTask_QuestTimer::OnStop()
{
	CountdownTimer.Shutdown();
	Super::OnStop();
}

void UExLatentTask_QuestTimer::BeginDestroy()
{
	CountdownTimer.Shutdown();
	Super::BeginDestroy();
}

int32 UExLatentTask_QuestTimer::ComputeObjectiveProgressFromElapsed() const
{
	const int32 TargetProgress = FMath::Max(1, FMath::RoundToInt(ResolvedDuration));
	return FMath::Clamp(FMath::FloorToInt(ElapsedTime), 0, TargetProgress);
}

void UExLatentTask_QuestTimer::SyncObjectiveProgressFromElapsed()
{
	if (!bSyncObjectiveProgress || !ObjectiveTag.IsValid() || !QuestTag.IsValid())
	{
		return;
	}

	const int32 NewProgress = ComputeObjectiveProgressFromElapsed();
	if (NewProgress == SyncedObjectiveProgress)
	{
		return;
	}

	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		return;
	}

	if (UExQuestReplicationComponent::RouteUpdateQuestObjective(WorldContext, QuestTag, ObjectiveTag, NewProgress))
	{
		SyncedObjectiveProgress = NewProgress;
	}
}

void UExLatentTask_QuestTimer::ResetObjectiveProgress()
{
	if (!ObjectiveTag.IsValid() || !QuestTag.IsValid())
	{
		return;
	}

	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		return;
	}

	if (UExQuestReplicationComponent::RouteUpdateQuestObjective(WorldContext, QuestTag, ObjectiveTag, 0))
	{
		SyncedObjectiveProgress = 0;
	}
}

void UExLatentTask_QuestTimer::ApplyQuestOnComplete_Implementation()
{
	if (!bSyncObjectiveProgress || !ObjectiveTag.IsValid())
	{
		Super::ApplyQuestOnComplete_Implementation();
		return;
	}

	if (!bCompletedNaturally)
	{
		return;
	}

	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		return;
	}

	UExQuestReplicationComponent::RouteCompleteQuestObjective(WorldContext, QuestTag, ObjectiveTag);
}
