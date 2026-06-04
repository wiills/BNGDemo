#include "Test/ScopedMessageTestListener.h"
#include "ScopedMessageSubsystem.h"

AScopedMessageTestListener::AScopedMessageTestListener()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AScopedMessageTestListener::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoSubscribeOnBeginPlay)
	{
		SubscribeToChannel();
	}
}

void AScopedMessageTestListener::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnsubscribeFromChannel();
	Super::EndPlay(EndPlayReason);
}

void AScopedMessageTestListener::SubscribeToChannel()
{
	if (!Channel.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("Listener %s: Channel is invalid"), *GetName());
		return;
	}

	UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(this);
	ListenerHandle = Subsystem.Subscribe<FScopedMessageTestPayload>(
		Channel,
		this,
		&AScopedMessageTestListener::OnMessageReceived,
		MatchType);

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] Listener %s: Subscribed to channel %s Scope=%s"),
		*UScopedMessageSubsystem::GetNetModePrefix(this), *GetName(), *Channel.ToString(), *Subsystem.ResolveScopeId(this).ToString());
}

void AScopedMessageTestListener::UnsubscribeFromChannel()
{
	if (ListenerHandle.IsValid())
	{
		ListenerHandle.Unregister();
		UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] Listener %s: Unsubscribed"), *UScopedMessageSubsystem::GetNetModePrefix(this), *GetName());
	}
}

void AScopedMessageTestListener::OnMessageReceived(FGameplayTag InChannel, const FScopedMessageTestPayload& Payload)
{
	ReceivedCount++;
	LastReceivedMessage = Payload.Message;

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("[%s] Listener %s: Received [%s] Counter=%d Location=%s (total: %d)"),
		*UScopedMessageSubsystem::GetNetModePrefix(this), *GetName(), *Payload.Message, Payload.Counter, *Payload.Location.ToString(), ReceivedCount);

	BP_OnMessageReceived(Payload.Message, Payload.Counter, Payload.Location);
}
