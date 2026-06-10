#include "ScopedMessagePoiRootActor.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "ScopedMessageScopeComponent.h"
#include "ScopedMessageSubsystem.h"

AScopedMessagePoiRootActor::AScopedMessagePoiRootActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	ScopeComponent = CreateDefaultSubobject<UScopedMessageScopeComponent>(TEXT("PoiScope"));
}

void AScopedMessagePoiRootActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRegisterPlayersOnBeginPlay && HasAuthority())
	{
		RegisterAllCurrentPlayers();
	}
}

void AScopedMessagePoiRootActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bAutoUnregisterPlayersOnEndPlay && HasAuthority())
	{
		UnregisterAllCurrentPlayers();
	}

	Super::EndPlay(EndPlayReason);
}

FScopedMessageScopeId AScopedMessagePoiRootActor::GetScopeId() const
{
	return ScopeComponent ? ScopeComponent->GetConfiguredScopeId() : FScopedMessageScopeId();
}

void AScopedMessagePoiRootActor::RegisterPlayer(APlayerController* PlayerController)
{
	if (HasAuthority() && PlayerController && UScopedMessageSubsystem::HasInstance(this))
	{
		const FScopedMessageScopeId ScopeId = GetScopeId();
		UScopedMessageSubsystem::Get(this).RegisterPlayerForScope(PlayerController, ScopeId);

		UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage Poi root %s registered player %s Scope=%s"),
			*UScopedMessageSubsystem::GetNetModePrefix(this),
			*GetName(),
			*GetNameSafe(PlayerController),
			*ScopeId.ToString());
	}
}

void AScopedMessagePoiRootActor::UnregisterPlayer(APlayerController* PlayerController)
{
	if (HasAuthority() && PlayerController && UScopedMessageSubsystem::HasInstance(this))
	{
		const FScopedMessageScopeId ScopeId = GetScopeId();
		UScopedMessageSubsystem::Get(this).UnregisterPlayerForScope(PlayerController, ScopeId);

		UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage Poi root %s unregistered player %s Scope=%s"),
			*UScopedMessageSubsystem::GetNetModePrefix(this),
			*GetName(),
			*GetNameSafe(PlayerController),
			*ScopeId.ToString());
	}
}

void AScopedMessagePoiRootActor::RegisterAllCurrentPlayers()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		RegisterPlayer(It->Get());
	}
}

void AScopedMessagePoiRootActor::UnregisterAllCurrentPlayers()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		UnregisterPlayer(It->Get());
	}
}
