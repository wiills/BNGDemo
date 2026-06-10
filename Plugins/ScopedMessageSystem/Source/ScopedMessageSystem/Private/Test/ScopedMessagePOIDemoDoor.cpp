#include "Test/ScopedMessagePoiDemoDoor.h"

#include "Net/UnrealNetwork.h"
#include "ScopedMessageSubsystem.h"

AScopedMessagePoiDemoDoor::AScopedMessagePoiDemoDoor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationChannel = FGameplayTag::RequestGameplayTag(TEXT("Poi.Demo.Terminal.Activated"), false);
}

void AScopedMessagePoiDemoDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromTerminal();
	Super::EndPlay(EndPlayReason);
}

void AScopedMessagePoiDemoDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AScopedMessagePoiDemoDoor, bIsOpen);
}

void AScopedMessagePoiDemoDoor::SubscribeToTerminal()
{
	if (!ActivationChannel.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("Poi demo door %s has invalid ActivationChannel"), *GetName());
		return;
	}

	UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(this);
	const FScopedMessageScopeId ResolvedScopeId = Subsystem.ResolveScopeId(this);
	if (!ResolvedScopeId.IsValid())
	{
		EnsurePoiScopeReady();
		return;
	}

	if (ListenerHandle.IsValid())
	{
		if (SubscribedScopeId == ResolvedScopeId)
		{
			return;
		}

		ListenerHandle.Unregister();
	}

	ListenerHandle = Subsystem.Subscribe<FScopedMessageDemoTerminalActivatedPayload>(
		ActivationChannel,
		this,
		&AScopedMessagePoiDemoDoor::OnTerminalActivated);
	SubscribedScopeId = ResolvedScopeId;

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] Poi demo door %s subscribed Scope=%s Channel=%s"),
		*UScopedMessageSubsystem::GetNetModePrefix(this),
		*GetName(),
		*SubscribedScopeId.ToString(),
		*ActivationChannel.ToString());
}

void AScopedMessagePoiDemoDoor::UnsubscribeFromTerminal()
{
	if (ListenerHandle.IsValid())
	{
		ListenerHandle.Unregister();
	}

	SubscribedScopeId = FScopedMessageScopeId();
}

void AScopedMessagePoiDemoDoor::OnPoiScopeReady(FScopedMessageScopeId ScopeId)
{
	Super::OnPoiScopeReady(ScopeId);

	if (bAutoSubscribeOnBeginPlay)
	{
		SubscribeToTerminal();
	}
}

void AScopedMessagePoiDemoDoor::ResetDoor()
{
	if (HasAuthority())
	{
		bIsOpen = false;
		OpenCount = 0;
	}
}

void AScopedMessagePoiDemoDoor::OnRep_IsOpen()
{
	if (bIsOpen)
	{
		BP_OnDoorOpened(RequiredTerminalId, OpenCount);
	}
}

void AScopedMessagePoiDemoDoor::OnTerminalActivated(FGameplayTag Channel, const FScopedMessageDemoTerminalActivatedPayload& Payload)
{
	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] Poi demo door %s received Channel=%s Scope=%s TerminalId=%s RequiredTerminalId=%s"),
		*UScopedMessageSubsystem::GetNetModePrefix(this),
		*GetName(),
		*Channel.ToString(),
		*UScopedMessageSubsystem::Get(this).ResolveScopeId(this).ToString(),
		*Payload.TerminalId.ToString(),
		*RequiredTerminalId.ToString());

	if (!RequiredTerminalId.IsNone() && Payload.TerminalId != RequiredTerminalId)
	{
		UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] Poi demo door %s ignored terminal activation because TerminalId did not match"),
			*UScopedMessageSubsystem::GetNetModePrefix(this),
			*GetName());
		return;
	}

	OpenDoor(Payload);
}

void AScopedMessagePoiDemoDoor::OpenDoor(const FScopedMessageDemoTerminalActivatedPayload& Payload)
{
	if (!bIsOpen)
	{
		bIsOpen = true;
	}

	OpenCount++;

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] Poi demo door %s opened by TerminalId=%s Count=%d"),
		*UScopedMessageSubsystem::GetNetModePrefix(this),
		*GetName(),
		*Payload.TerminalId.ToString(),
		Payload.ActivationCount);

	BP_OnDoorOpened(Payload.TerminalId, Payload.ActivationCount);
}
