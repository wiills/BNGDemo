// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExLatentTask_QuestTimer.h"

#include "Quest/ExQuestBlueprintLibrary.h"
#include "Quest/ExQuestManagerSubsystem.h"
#include "Quest/ExQuestReplicationComponent.h"
#include "TimerManager.h"

void UExLatentTask_QuestTimer::OnStart()
{
	ElapsedTime = 0.0f;
	SyncedObjectiveProgress = 0;
	bCompletedNaturally = false;
	bCountdownActive = false;
	bProgressRollingBack = false;
	ResolvedDuration = 0.0f;

	Super::OnStart();

	if (!ResolveDuration())
	{
		UE_LOG(LogTemp, Warning, TEXT("UExLatentTask_QuestTimer: failed to resolve duration for task '%s' objective '%s'."),
			*QuestTag.ToString(), *ObjectiveTag.ToString());
	}

	RemainingTime = ResolvedDuration;

	if (bStartTimerOnStart)
	{
		StartCountdownInternal(true);
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
	if (Duration > 0.0f)
	{
		ResolvedDuration = Duration;
		return true;
	}

	int32 TargetProgress = 0;
	if (TryGetObjectiveTargetProgress(TargetProgress))
	{
		ResolvedDuration = static_cast<float>(TargetProgress);
		return ResolvedDuration > 0.0f;
	}

	return false;
}

void UExLatentTask_QuestTimer::StartCountdown()
{
	if (!IsRunning())
	{
		return;
	}

	if (bCountdownActive && !bProgressRollingBack)
	{
		return;
	}

	ClearCountdownTimer();
	bProgressRollingBack = false;
	StartCountdownInternal(ElapsedTime <= 0.0f);
}

void UExLatentTask_QuestTimer::PauseCountdown()
{
	if (!IsRunning() || !bCountdownActive)
	{
		return;
	}

	ClearCountdownTimer();
}

void UExLatentTask_QuestTimer::ResumeCountdown()
{
	if (!IsRunning() || bCountdownActive)
	{
		return;
	}

	if (bProgressRollingBack)
	{
		if (ElapsedTime <= 0.0f)
		{
			StopProgressRollback(bResetProgressOnReset);
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

void UExLatentTask_QuestTimer::RestartCountdown()
{
	if (!IsRunning())
	{
		return;
	}

	bProgressRollingBack = false;
	ClearCountdownTimer();
	StartCountdownInternal(true);
}

void UExLatentTask_QuestTimer::ResetCountdown()
{
	if (!IsRunning())
	{
		return;
	}

	if (bRollbackOnReset && ElapsedTime > 0.0f)
	{
		StartProgressRollbackInternal();
		return;
	}

	ResetCountdownElapsed(bResetProgressOnReset);
}

void UExLatentTask_QuestTimer::StartProgressRollback()
{
	if (!IsRunning())
	{
		return;
	}

	StartProgressRollbackInternal();
}

bool UExLatentTask_QuestTimer::IsCountdownPaused() const
{
	return IsRunning()
		&& !bCountdownActive
		&& !bProgressRollingBack
		&& ElapsedTime > 0.0f
		&& ElapsedTime < ResolvedDuration;
}

bool UExLatentTask_QuestTimer::IsProgressRollbackPaused() const
{
	return IsRunning() && bProgressRollingBack && !bCountdownActive && ElapsedTime > 0.0f;
}

void UExLatentTask_QuestTimer::StartCountdownInternal(bool bResetElapsed)
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

	if (ResolvedDuration <= 0.0f)
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

void UExLatentTask_QuestTimer::ResetCountdownElapsed(bool bResetObjectiveProgress)
{
	ClearCountdownTimer();
	bProgressRollingBack = false;
	ElapsedTime = 0.0f;
	RemainingTime = ResolvedDuration;
	SyncedObjectiveProgress = 0;

	if (bSyncObjectiveProgress && bResetObjectiveProgress && ObjectiveTag.IsValid())
	{
		ResetObjectiveProgress();
	}
}

void UExLatentTask_QuestTimer::StartProgressRollbackInternal()
{
	if (ElapsedTime <= 0.0f)
	{
		return;
	}

	ClearCountdownTimer();
	bProgressRollingBack = true;
	BeginCountdownTimer();
}

void UExLatentTask_QuestTimer::StopProgressRollback(bool bResetObjectiveProgress)
{
	ClearCountdownTimer();
	bProgressRollingBack = false;
	ElapsedTime = 0.0f;
	RemainingTime = ResolvedDuration;
	SyncedObjectiveProgress = 0;

	if (bSyncObjectiveProgress && bResetObjectiveProgress && ObjectiveTag.IsValid())
	{
		ResetObjectiveProgress();
	}
}

void UExLatentTask_QuestTimer::BeginCountdownTimer()
{
	if (bCountdownActive || !IsRunning())
	{
		return;
	}

	if (bProgressRollingBack)
	{
		if (ElapsedTime <= 0.0f)
		{
			StopProgressRollback(bResetProgressOnReset);
			return;
		}
	}
	else if (ElapsedTime >= ResolvedDuration)
	{
		CompleteCountdown();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	RemainingTime = FMath::Max(0.0f, ResolvedDuration - ElapsedTime);
	bCountdownActive = true;

	const float FirstDelay = bProgressRollingBack
		? TickInterval
		: FMath::Min(TickInterval, RemainingTime);
	World->GetTimerManager().SetTimer(
		CountdownTickHandle,
		this,
		bProgressRollingBack ? &UExLatentTask_QuestTimer::HandleProgressRollbackTick : &UExLatentTask_QuestTimer::HandleTimerTick,
		TickInterval,
		true,
		FirstDelay);
}

void UExLatentTask_QuestTimer::OnStop()
{
	ClearCountdownTimer();
	Super::OnStop();
}

void UExLatentTask_QuestTimer::BeginDestroy()
{
	ClearCountdownTimer();
	Super::BeginDestroy();
}

void UExLatentTask_QuestTimer::InterruptTimer()
{
	if (!IsRunning())
	{
		return;
	}

	ClearCountdownTimer();
	bProgressRollingBack = false;
	SyncObjectiveProgressFromElapsed();
	TryStop();
}

void UExLatentTask_QuestTimer::CancelTimer()
{
	if (!IsRunning() && !IsPending())
	{
		return;
	}

	ClearCountdownTimer();
	bProgressRollingBack = false;

	if (bSyncObjectiveProgress && bResetProgressOnCancel && ObjectiveTag.IsValid())
	{
		ResetObjectiveProgress();
	}

	Terminate();
}

void UExLatentTask_QuestTimer::HandleTimerTick()
{
	if (!IsRunning() || bProgressRollingBack)
	{
		return;
	}

	ElapsedTime = FMath::Min(ElapsedTime + TickInterval, ResolvedDuration);
	RemainingTime = FMath::Max(0.0f, ResolvedDuration - ElapsedTime);

	SyncObjectiveProgressFromElapsed();
	ReceiveOnTimerTick(RemainingTime, ElapsedTime);

	if (ElapsedTime >= ResolvedDuration)
	{
		CompleteCountdown();
	}
}

void UExLatentTask_QuestTimer::HandleProgressRollbackTick()
{
	if (!IsRunning() || !bProgressRollingBack)
	{
		return;
	}

	const float RollbackDelta = RollbackRate * TickInterval;
	ElapsedTime = FMath::Max(0.0f, ElapsedTime - RollbackDelta);
	RemainingTime = FMath::Max(0.0f, ResolvedDuration - ElapsedTime);

	SyncObjectiveProgressFromElapsed();
	ReceiveOnTimerTick(RemainingTime, ElapsedTime);

	if (ElapsedTime <= 0.0f)
	{
		StopProgressRollback(bResetProgressOnReset);
	}
}

void UExLatentTask_QuestTimer::ClearCountdownTimer()
{
	bCountdownActive = false;

	if (CountdownTickHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(CountdownTickHandle);
		}
		CountdownTickHandle.Invalidate();
	}
}

void UExLatentTask_QuestTimer::CompleteCountdown()
{
	if (!IsRunning())
	{
		return;
	}

	ClearCountdownTimer();
	RemainingTime = 0.0f;
	ElapsedTime = ResolvedDuration;
	bCompletedNaturally = true;
	SyncObjectiveProgressFromElapsed();
	TryStop();
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
