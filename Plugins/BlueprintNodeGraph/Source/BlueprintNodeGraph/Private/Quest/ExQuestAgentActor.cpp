// Copyright Epic Games, Inc. All Rights Reserved.

#include "Quest/ExQuestAgentActor.h"

#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/ExQuestReplicationComponent.h"
#include "TimerManager.h"

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

	if (HasAuthority() && bAutoFireServerReadyEvent)
	{
		EventOnServerReady();
	}

	if (bAutoFireClientReadyEvent)
	{
		StartClientReadyPolling();
	}
}

void AExQuestAgentActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopClientReadyPolling();
	Super::EndPlay(EndPlayReason);
}

void AExQuestAgentActor::LoadQuestFromConfiguredAsset()
{
	if (!QuestDataAsset || !HasAuthority())
	{
		return;
	}

	UExQuestReplicationComponent::EnsureOnGameState(this);
	UExQuestReplicationComponent::RouteLoadQuestFromAsset(this, QuestDataAsset, bPreserveRuntime);
}

void AExQuestAgentActor::NotifyClientReadyWhenAvailable()
{
	ClientReadyElapsedSeconds = 0.0f;
	TryFireClientReadyEvent();

	if (!NotifiedClientPlayerController && bAutoFireClientReadyEvent)
	{
		StartClientReadyPolling();
	}
}

void AExQuestAgentActor::StartClientReadyPolling()
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	StopClientReadyPolling();
	ClientReadyElapsedSeconds = 0.0f;

	TryFireClientReadyEvent();
	if (NotifiedClientPlayerController)
	{
		return;
	}

	const float Interval = FMath::Max(ClientReadyPollInterval, 0.01f);
	World->GetTimerManager().SetTimer(
		ClientReadyTimerHandle,
		this,
		&AExQuestAgentActor::TryFireClientReadyEvent,
		Interval,
		true);
}

void AExQuestAgentActor::StopClientReadyPolling()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ClientReadyTimerHandle);
	}
}

void AExQuestAgentActor::TryFireClientReadyEvent()
{
	if (NotifiedClientPlayerController)
	{
		StopClientReadyPolling();
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		StopClientReadyPolling();
		return;
	}

	if (ClientReadyMaxWaitSeconds > 0.0f)
	{
		ClientReadyElapsedSeconds += FMath::Max(ClientReadyPollInterval, 0.01f);
		if (ClientReadyElapsedSeconds > ClientReadyMaxWaitSeconds)
		{
			StopClientReadyPolling();
			return;
		}
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (!PlayerController || !PlayerController->IsLocalController())
	{
		return;
	}

	NotifiedClientPlayerController = PlayerController;
	StopClientReadyPolling();
	EventOnClientReady(PlayerController);
}
