#include "ScopedMessageTestBroadcaster.h"
#include "ScopedMessageSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

AScopedMessageTestBroadcaster::AScopedMessageTestBroadcaster()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AScopedMessageTestBroadcaster::BroadcastTestMessage()
{
	if (!Channel.IsValid())
	{
		UE_LOG(LogScopedMessageSubsystem, Warning, TEXT("Broadcaster %s: Channel is invalid"), *GetName());
		return;
	}

	UObject* Context = ScopeContext ? ScopeContext : this;

	FScopedMessageTestPayload Payload;
	Payload.Message = MessageText;
	Payload.Counter = ++BroadcastCounter;
	Payload.Location = GetActorLocation();

	UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(this);
	Subsystem.BroadcastMessage(Channel, Payload, Context, Replication);

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("Broadcaster %s: Sent [%s] Counter=%d Scope=%s"),
		*GetName(), *Payload.Message, Payload.Counter, *Subsystem.ResolveScopeId(Context).ToString());
}

void AScopedMessageTestBroadcaster::StartAutoBroadcast(float Interval)
{
	GetWorldTimerManager().SetTimer(AutoBroadcastTimer, this, &AScopedMessageTestBroadcaster::OnAutoBroadcast, Interval, true);
}

void AScopedMessageTestBroadcaster::StopAutoBroadcast()
{
	GetWorldTimerManager().ClearTimer(AutoBroadcastTimer);
}

void AScopedMessageTestBroadcaster::OnAutoBroadcast()
{
	BroadcastTestMessage();
}
