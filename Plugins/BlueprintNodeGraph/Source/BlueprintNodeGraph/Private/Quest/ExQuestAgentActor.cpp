// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestAgentActor.h"

#include "Components/SceneComponent.h"
#include "Quest/ExQuestDefinition.h"
#include "Quest/ExQuestReplicationComponent.h"

AExQuestAgentActor::AExQuestAgentActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bAlwaysRelevant = true;
	bNetUseOwnerRelevancy = false;
	bNetLoadOnClient = true;
	SetNetDormancy(ENetDormancy::DORM_Never);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AExQuestAgentActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoLoadOnBeginPlay)
	{
		LoadQuestFromConfiguredAsset();
	}
}

void AExQuestAgentActor::LoadQuestFromConfiguredAsset()
{
	if (!QuestDataAsset)
	{
		return;
	}

	if (!HasAuthority())
	{
		return;
	}

	UExQuestReplicationComponent::RouteLoadQuestFromAsset(this, QuestDataAsset, bPreserveRuntime);
}
