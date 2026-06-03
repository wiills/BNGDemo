#include "ScopedMessageTestListener.h"
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

	UObject* Context = ScopeContext ? ScopeContext : this;

	UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(this);
	ListenerHandle = Subsystem.Subscribe<FScopedMessageTestPayload>(
		Channel,
		this,
		&AScopedMessageTestListener::OnMessageReceived,
		Context,
		MatchType);

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("Listener %s: Subscribed to channel %s Scope=%s"),
		*GetName(), *Channel.ToString(), *Subsystem.ResolveScopeId(Context).ToString());
}

void AScopedMessageTestListener::UnsubscribeFromChannel()
{
	if (ListenerHandle.IsValid())
	{
		ListenerHandle.Unregister();
		UE_LOG(LogScopedMessageSubsystem, Log, TEXT("Listener %s: Unsubscribed"), *GetName());
	}
}

void AScopedMessageTestListener::OnMessageReceived(FGameplayTag InChannel, const FScopedMessageTestPayload& Payload)
{
	ReceivedCount++;
	LastReceivedMessage = Payload.Message;

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("Listener %s: Received [%s] Counter=%d Location=%s (total: %d)"),
		*GetName(), *Payload.Message, Payload.Counter, *Payload.Location.ToString(), ReceivedCount);

	BP_OnMessageReceived(Payload.Message, Payload.Counter, Payload.Location);
}
