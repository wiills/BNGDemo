// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/LatentTasks/ExLatentTask_Quest.h"

#include "Quest/ExQuestReplicationComponent.h"
#include "Kismet/GameplayStatics.h"

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
			ProgressDeltaOnStop);
		break;
	}
}
