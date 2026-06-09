#include "Test/ScopedMessagePOIDemoTerminal.h"

#include "ScopedMessageSubsystem.h"

AScopedMessagePOIDemoTerminal::AScopedMessagePOIDemoTerminal()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	ActivationChannel = FGameplayTag::RequestGameplayTag(TEXT("POI.Demo.Terminal.Activated"), false);
}

void AScopedMessagePOIDemoTerminal::ActivateTerminal()
{
	if (!HasAuthority())
	{
		Server_ActivateTerminal();
		return;
	}

	if (!ActivationChannel.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("POI demo terminal %s has invalid ActivationChannel"), *GetName());
		return;
	}

	FScopedMessageDemoTerminalActivatedPayload Payload;
	Payload.TerminalId = TerminalId;
	Payload.ActivationCount = ++ActivationCount;
	Payload.Location = GetActorLocation();

	UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(this);
	Subsystem.BroadcastMessage(this, ActivationChannel, Payload, Replication);

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] POI demo terminal %s activated TerminalId=%s Count=%d Scope=%s"),
		*UScopedMessageSubsystem::GetNetModePrefix(this),
		*GetName(),
		*TerminalId.ToString(),
		ActivationCount,
		*Subsystem.ResolveScopeId(this).ToString());
}

void AScopedMessagePOIDemoTerminal::Server_ActivateTerminal_Implementation()
{
	ActivateTerminal();
}
