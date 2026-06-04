#include "Test/ScopedMessageTestBroadcaster.h"
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

	FScopedMessageTestPayload Payload;
	Payload.Message = MessageText;
	Payload.Counter = ++BroadcastCounter;
	Payload.Location = GetActorLocation();

	UScopedMessageSubsystem& Subsystem = UScopedMessageSubsystem::Get(this);
	Subsystem.BroadcastMessage(this, Channel, Payload, ScopeContext, Replication);

	UE_LOG(LogScopedMessageSubsystem, Log, TEXT("Broadcaster %s: Sent [%s] Counter=%d Scope=%s"),
		*GetName(), *Payload.Message, Payload.Counter, *Subsystem.ResolveScopeId(ScopeContext ? ScopeContext : this).ToString());
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
