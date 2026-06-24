// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/LatentTasks/ExLatentTask_Quest.h"

#include "Quest/ExQuestManagerSubsystem.h"
#include "Quest/ExQuestReplicationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"

UExLatentTask_Quest* UExLatentTask_Quest::CreateQuestProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_Quest> Class)
{
	if (!Class || !WorldContextObject)
	{
		return nullptr;
	}

	if (!Class->IsChildOf(UExLatentTask_Quest::StaticClass()))
	{
		return nullptr;
	}

	return Cast<UExLatentTask_Quest>(UGameplayStatics::SpawnObject(Class, WorldContextObject));
}

bool UExLatentTask_Quest::IsQuestLatentClass(TSubclassOf<UExLatentTask_Quest> Class)
{
	return Class && Class->IsChildOf(UExLatentTask_Quest::StaticClass());
}

bool UExLatentTask_Quest::UpdateQuestObjectiveProgress(int32 NewProgress)
{
	if (!QuestTag.IsValid() || !ObjectiveTag.IsValid())
	{
		return false;
	}

	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		return false;
	}

	return UExQuestReplicationComponent::RouteUpdateQuestObjective(WorldContext, QuestTag, ObjectiveTag, NewProgress);
}

bool UExLatentTask_Quest::UpdateQuestObjectiveProgressFloat(float NewProgress)
{
	if (!QuestTag.IsValid() || !ObjectiveTag.IsValid())
	{
		return false;
	}

	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		return false;
	}

	return UExQuestReplicationComponent::RouteUpdateQuestObjectiveFloat(WorldContext, QuestTag, ObjectiveTag, NewProgress);
}

bool UExLatentTask_Quest::IncrementQuestObjectiveProgress()
{
	if (!QuestTag.IsValid() || !ObjectiveTag.IsValid())
	{
		return false;
	}

	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		return false;
	}

	return UExQuestReplicationComponent::RouteIncrementQuestObjective(WorldContext, QuestTag, ObjectiveTag, 1);
}

bool UExLatentTask_Quest::GetQuestObjectiveProgress(int32& OutCurrentProgress, int32& OutTargetProgress) const
{
	OutCurrentProgress = 0;
	OutTargetProgress = 0;

	if (!QuestTag.IsValid() || !ObjectiveTag.IsValid())
	{
		return false;
	}

	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	const UExQuestManagerSubsystem* Manager = GameInstance ? GameInstance->GetSubsystem<UExQuestManagerSubsystem>() : nullptr;
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
			OutCurrentProgress = Objective.CurrentProgress;
			OutTargetProgress = Objective.TargetProgress;
			return true;
		}
	}

	return false;
}

bool UExLatentTask_Quest::GetQuestObjectiveProgressFloat(float& OutCurrentProgress, float& OutTargetProgress) const
{
	OutCurrentProgress = 0.f;
	OutTargetProgress = 0.f;

	if (!QuestTag.IsValid() || !ObjectiveTag.IsValid())
	{
		return false;
	}

	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	const UExQuestManagerSubsystem* Manager = GameInstance ? GameInstance->GetSubsystem<UExQuestManagerSubsystem>() : nullptr;
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
			OutCurrentProgress = Objective.CurrentProgressFloat;
			OutTargetProgress = static_cast<float>(Objective.TargetProgress);
			return true;
		}
	}

	return false;
}

bool UExLatentTask_Quest::GetQuestObjectiveTargetProgress(int32& OutTargetProgress) const
{
	int32 CurrentProgress = 0;
	return GetQuestObjectiveProgress(CurrentProgress, OutTargetProgress);
}



void UExLatentTask_Quest::OnStart()
{
	if (bAutoEnsureQuestActiveOnStart)
	{
		EnsureQuestTaskActive();
	}

	Super::OnStart();
}

void UExLatentTask_Quest::OnStop()
{
	if (bApplyQuestOnSuccessfulStop && GetState() == EExLatentTaskState::Completed)
	{
		ApplyQuestOnComplete();
	}

	Super::OnStop();
}

void UExLatentTask_Quest::EnsureQuestTaskActive()
{
	if (!QuestTag.IsValid())
	{
		return;
	}

	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		return;
	}

	UExQuestReplicationComponent::RouteUnlockQuest(WorldContext, QuestTag);
	UExQuestReplicationComponent::RouteActivateQuest(WorldContext, QuestTag);
}

void UExLatentTask_Quest::ApplyQuestOnComplete_Implementation()
{
	if (!QuestTag.IsValid())
	{
		return;
	}

	UObject* WorldContext = GetWorld();
	if (!WorldContext)
	{
		return;
	}

	if (!ObjectiveTag.IsValid())
	{
		UExQuestReplicationComponent::RouteCompleteQuest(WorldContext, QuestTag);
		return;
	}

	switch (CompleteAction)
	{
	case EExQuestCompleteAction::CompleteObjective:
		UExQuestReplicationComponent::RouteCompleteQuestObjective(WorldContext, QuestTag, ObjectiveTag);
		break;
	case EExQuestCompleteAction::IncrementProgress:
	default:
		UExQuestReplicationComponent::RouteIncrementQuestObjective(
			WorldContext,
			QuestTag,
			ObjectiveTag,
			ProgressDelta);
		break;
	}
}
