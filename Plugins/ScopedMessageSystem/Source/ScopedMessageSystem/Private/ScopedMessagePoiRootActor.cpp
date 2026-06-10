#include "ScopedMessagePoiRootActor.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "ScopedMessageScopeComponent.h"
#include "ScopedMessageSubsystem.h"

AScopedMessagePoiRootActor::AScopedMessagePoiRootActor()
{
	ScopeComponent = CreateDefaultSubobject<UScopedMessageScopeComponent>(TEXT("PoiScope"));
}

void AScopedMessagePoiRootActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PlayerRegistrationRetryTimer);
	}

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
		if (!ScopeId.IsValid())
		{
			UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage Poi root %s skipped player %s registration because Scope is not valid yet"),
				*UScopedMessageSubsystem::GetNetModePrefix(this),
				*GetName(),
				*GetNameSafe(PlayerController));
			return;
		}

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

void AScopedMessagePoiRootActor::EnsurePlayerRegistrationReady()
{
	if (!HasAuthority())
	{
		return;
	}

	const FScopedMessageScopeId ScopeId = GetScopeId();
	UWorld* World = GetWorld();
	bool bHasPlayerController = false;
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (It->Get())
			{
				bHasPlayerController = true;
				break;
			}
		}
	}

	if (!ScopeId.IsValid() || !bHasPlayerController)
	{
		UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage Poi root %s waiting before player registration Scope=%s HasPlayers=%s"),
			*UScopedMessageSubsystem::GetNetModePrefix(this),
			*GetName(),
			*ScopeId.ToString(),
			bHasPlayerController ? TEXT("true") : TEXT("false"));

		if (World)
		{
			World->GetTimerManager().SetTimer(
				PlayerRegistrationRetryTimer,
				this,
				&AScopedMessagePoiRootActor::EnsurePlayerRegistrationReady,
				FMath::Max(PlayerRegistrationRetryInterval, 0.01f),
				true);
		}
		return;
	}

	RegisterAllCurrentPlayers();

	if (World)
	{
		World->GetTimerManager().ClearTimer(PlayerRegistrationRetryTimer);
	}

	OnPlayerRegistrationReady(ScopeId);
}

FString AScopedMessagePoiRootActor::GetPoiActorLogLabel() const
{
	return TEXT("Poi root");
}

void AScopedMessagePoiRootActor::OnPoiScopeReady(FScopedMessageScopeId ScopeId)
{
	Super::OnPoiScopeReady(ScopeId);

	if (bAutoRegisterPlayersOnBeginPlay && HasAuthority())
	{
		EnsurePlayerRegistrationReady();
	}
}

void AScopedMessagePoiRootActor::OnPlayerRegistrationReady(FScopedMessageScopeId ScopeId)
{
	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] ScopedMessage Poi root %s player registration ready Scope=%s"),
		*UScopedMessageSubsystem::GetNetModePrefix(this),
		*GetName(),
		*ScopeId.ToString());
}
