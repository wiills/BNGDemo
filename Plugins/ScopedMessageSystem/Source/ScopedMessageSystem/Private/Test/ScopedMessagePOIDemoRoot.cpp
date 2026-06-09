#include "Test/ScopedMessagePOIDemoRoot.h"

#include "ScopedMessageScopeComponent.h"
#include "ScopedMessageSubsystem.h"

AScopedMessagePOIDemoRoot::AScopedMessagePOIDemoRoot()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	ScopeComponent = CreateDefaultSubobject<UScopedMessageScopeComponent>(TEXT("POIScope"));
}

FScopedMessageScopeId AScopedMessagePOIDemoRoot::GetScopeId() const
{
	return ScopeComponent ? ScopeComponent->GetConfiguredScopeId() : FScopedMessageScopeId();
}

void AScopedMessagePOIDemoRoot::RegisterPlayer(APlayerController* PlayerController)
{
	if (HasAuthority() && PlayerController && UScopedMessageSubsystem::HasInstance(this))
	{
		UScopedMessageSubsystem::Get(this).RegisterPlayerForScope(PlayerController, GetScopeId());
	}
}

void AScopedMessagePOIDemoRoot::UnregisterPlayer(APlayerController* PlayerController)
{
	if (HasAuthority() && PlayerController && UScopedMessageSubsystem::HasInstance(this))
	{
		UScopedMessageSubsystem::Get(this).UnregisterPlayerForScope(PlayerController, GetScopeId());
	}
}
