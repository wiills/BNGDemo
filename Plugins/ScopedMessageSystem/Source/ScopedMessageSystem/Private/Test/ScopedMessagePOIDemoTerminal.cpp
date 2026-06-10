#include "Test/ScopedMessagePoiDemoTerminal.h"

#include "ScopedMessageSubsystem.h"

AScopedMessagePoiDemoTerminal::AScopedMessagePoiDemoTerminal()
{
	ActivationChannel = FGameplayTag::RequestGameplayTag(TEXT("Poi.Demo.Terminal.Activated"), false);
}

void AScopedMessagePoiDemoTerminal::ActivateTerminal()
{
	if (!HasAuthority())
	{
		Server_ActivateTerminal();
		return;
	}

	if (!ActivationChannel.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("Poi demo terminal %s has invalid ActivationChannel"), *GetName());
		return;
	}

	FScopedMessageDemoTerminalActivatedPayload Payload;
	Payload.TerminalId = TerminalId;
	Payload.ActivationCount = ++ActivationCount;
	Payload.Location = GetActorLocation();

	UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(this);
	Subsystem.BroadcastMessage(this, ActivationChannel, Payload, Replication);

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] Poi demo terminal %s activated TerminalId=%s Count=%d Scope=%s"),
		*UScopedMessageSubsystem::GetNetModePrefix(this),
		*GetName(),
		*TerminalId.ToString(),
		ActivationCount,
		*Subsystem.ResolveScopeId(this).ToString());
}

void AScopedMessagePoiDemoTerminal::Server_ActivateTerminal_Implementation()
{
	ActivateTerminal();
}
