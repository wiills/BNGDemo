#include "ScopedMessageReplicatorComponent.h"
#include "ScopedMessageSubsystem.h"

UScopedMessageReplicatorComponent::UScopedMessageReplicatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UScopedMessageReplicatorComponent::NetMulticast_BroadcastMessage_Implementation(
	FGameplayTag Channel,
	FName ScopeIdName,
	const UScriptStruct* PayloadType,
	const TArray<uint8>& PayloadBytes)
{
	// Route the replicated message to the local ScopedMessageSubsystem instance on the client.
	if (UScopedMessageSubsystem::HasInstance(this))
	{
		UScopedMessageSubsystem::Get(this).HandleReplicatedMessage(Channel, ScopeIdName, PayloadType, PayloadBytes);
	}
}
