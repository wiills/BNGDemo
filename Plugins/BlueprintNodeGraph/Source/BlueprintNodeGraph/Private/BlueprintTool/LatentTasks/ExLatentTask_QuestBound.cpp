// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/LatentTasks/ExLatentTask_QuestBound.h"

#include "Quest/ExQuestManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"

UExLatentTask_QuestBound* UExLatentTask_QuestBound::CreateQuestBoundProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_QuestBound> Class)
{
	if (!Class || !WorldContextObject)
	{
		return nullptr;
	}

	if (!Class->IsChildOf(UExLatentTask_QuestBound::StaticClass()))
	{
		return nullptr;
	}

	return Cast<UExLatentTask_QuestBound>(UGameplayStatics::SpawnObject(Class, WorldContextObject));
}

bool UExLatentTask_QuestBound::IsQuestBoundLatentClass(TSubclassOf<UExLatentTask_QuestBound> Class)
{
	return Class && Class->IsChildOf(UExLatentTask_QuestBound::StaticClass());
}

void UExLatentTask_QuestBound::OnStop()
{
	if (bApplyQuestOnSuccessfulStop && GetState() == EExLatentTaskState::Completed)
	{
		ApplyQuestBindingOnComplete();
	}

	Super::OnStop();
}

void UExLatentTask_QuestBound::ApplyQuestBindingOnComplete()
{
	if (!BoundQuestTag.IsValid() || !BoundObjectiveTag.IsValid())
	{
		return;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);
	if (!GameInstance)
	{
		return;
	}

	UExQuestManagerSubsystem* QuestManager = GameInstance->GetSubsystem<UExQuestManagerSubsystem>();
	if (!QuestManager)
	{
		return;
	}

	switch (CompleteAction)
	{
	case EExQuestBoundCompleteAction::CompleteObjective:
		QuestManager->CompleteQuestObjective(BoundQuestTag, BoundObjectiveTag);
		break;
	case EExQuestBoundCompleteAction::IncrementProgress:
	default:
		QuestManager->IncrementQuestObjective(BoundQuestTag, BoundObjectiveTag, ProgressDeltaOnStop);
		break;
	}
}
