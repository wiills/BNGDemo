#include "ScopedMessageTypes.h"
#include "ScopedMessageSubsystem.h"

void FScopedMessageListenerHandle::Unregister()
{
	if (UScopedMessageSubsystem* StrongSubsystem = Subsystem.Get())
	{
		StrongSubsystem->Unsubscribe(*this);
	}
}
