#include "ScopedMessageTypes.h"
#include "ScopedMessageSubsystem.h"

FScopedMessageListenerHandle::FScopedMessageListenerHandle(
	UScopedMessageSubsystem* InSubsystem,
	FScopedMessageScopeId InScopeId,
	FGameplayTag InChannel,
	int32 InID)
	: Subsystem(InSubsystem)
	, ScopeId(InScopeId)
	, Channel(InChannel)
	, HandleID(InID)
{
}

void FScopedMessageListenerHandle::Unregister()
{
	if (UScopedMessageSubsystem* StrongSubsystem = Subsystem.Get())
	{
		StrongSubsystem->Unsubscribe(*this);
	}
}
