// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExLatentTask_QuestTimer.h"

#include "Quest/ExQuestReplicationComponent.h"
#include "TimerManager.h"

void UExLatentTask_QuestTimer::OnStart()
{
	RemainingTime = Duration;
	ElapsedTime = 0.0f;
	SyncedObjectiveProgress = 0;
	bCompletedNaturally = false;

	if (bSyncObjectiveProgress && bResetProgressOnStart && ObjectiveTag.IsValid())
	{
		ResetObjectiveProgress();
	}

	Super::OnStart();

	if (Duration <= 0.0f)
	{
		CompleteCountdown();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float FirstDelay = FMath::Min(TickInterval, Duration);
	World->GetTimerManager().SetTimer(
		CountdownTickHandle,
		this,
		&UExLatentTask_QuestTimer::HandleTimerTick,
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

	if (bSyncObjectiveProgress && bResetProgressOnCancel && ObjectiveTag.IsValid())
	{
		ResetObjectiveProgress();
	}

	Terminate();
}

void UExLatentTask_QuestTimer::HandleTimerTick()
{
	if (!IsRunning())
	{
		return;
	}

	ElapsedTime = FMath::Min(ElapsedTime + TickInterval, Duration);
	RemainingTime = FMath::Max(0.0f, Duration - ElapsedTime);

	SyncObjectiveProgressFromElapsed();
	ReceiveOnTimerTick(RemainingTime, ElapsedTime);

	if (ElapsedTime >= Duration)
	{
		CompleteCountdown();
	}
}

void UExLatentTask_QuestTimer::ClearCountdownTimer()
{
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
	ElapsedTime = Duration;
	bCompletedNaturally = true;
	SyncObjectiveProgressFromElapsed();
	TryStop();
}

int32 UExLatentTask_QuestTimer::ComputeObjectiveProgressFromElapsed() const
{
	const int32 TargetProgress = FMath::Max(1, FMath::RoundToInt(Duration));
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
