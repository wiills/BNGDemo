#include "Test/ScopedMessagePOIDemoDoor.h"

#include "Net/UnrealNetwork.h"
#include "ScopedMessageSubsystem.h"

AScopedMessagePOIDemoDoor::AScopedMessagePOIDemoDoor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;

	ActivationChannel = FGameplayTag::RequestGameplayTag(TEXT("POI.Demo.Terminal.Activated"), false);
}

void AScopedMessagePOIDemoDoor::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoSubscribeOnBeginPlay)
	{
		SubscribeToTerminal();
	}
}

void AScopedMessagePOIDemoDoor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromTerminal();
	Super::EndPlay(EndPlayReason);
}

void AScopedMessagePOIDemoDoor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AScopedMessagePOIDemoDoor, bIsOpen);
}

void AScopedMessagePOIDemoDoor::SubscribeToTerminal()
{
	if (ListenerHandle.IsValid())
	{
		return;
	}

	if (!ActivationChannel.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("POI demo door %s has invalid ActivationChannel"), *GetName());
		return;
	}

	UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(this);
	ListenerHandle = Subsystem.Subscribe<FScopedMessageDemoTerminalActivatedPayload>(
		ActivationChannel,
		this,
		&AScopedMessagePOIDemoDoor::OnTerminalActivated);

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] POI demo door %s subscribed Scope=%s Channel=%s"),
		*UScopedMessageSubsystem::GetNetModePrefix(this),
		*GetName(),
		*Subsystem.ResolveScopeId(this).ToString(),
		*ActivationChannel.ToString());
}

void AScopedMessagePOIDemoDoor::UnsubscribeFromTerminal()
{
	if (ListenerHandle.IsValid())
	{
		ListenerHandle.Unregister();
	}
}

void AScopedMessagePOIDemoDoor::ResetDoor()
{
	if (HasAuthority())
	{
		bIsOpen = false;
		OpenCount = 0;
	}
}

void AScopedMessagePOIDemoDoor::OnRep_IsOpen()
{
	if (bIsOpen)
	{
		BP_OnDoorOpened(RequiredTerminalId, OpenCount);
	}
}

void AScopedMessagePOIDemoDoor::OnTerminalActivated(FGameplayTag Channel, const FScopedMessageDemoTerminalActivatedPayload& Payload)
{
	if (!RequiredTerminalId.IsNone() && Payload.TerminalId != RequiredTerminalId)
	{
		return;
	}

	OpenDoor(Payload);
}

void AScopedMessagePOIDemoDoor::OpenDoor(const FScopedMessageDemoTerminalActivatedPayload& Payload)
{
	if (!bIsOpen)
	{
		bIsOpen = true;
	}

	OpenCount++;

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] POI demo door %s opened by TerminalId=%s Count=%d"),
		*UScopedMessageSubsystem::GetNetModePrefix(this),
		*GetName(),
		*Payload.TerminalId.ToString(),
		Payload.ActivationCount);

	BP_OnDoorOpened(Payload.TerminalId, Payload.ActivationCount);
}
