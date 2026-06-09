#include "ScopedMessageReplicatorComponent.h"

#include "Engine/World.h"
#include "ScopedMessageSubsystem.h"

UScopedMessageReplicatorComponent::UScopedMessageReplicatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UScopedMessageReplicatorComponent::NetMulticast_BroadcastMessage_Implementation(const FScopedMessageNetworkPacket& Packet)
{
	UWorld* World = GetWorld();
	if (World && World->GetNetMode() == NM_Client && UScopedMessageSubsystem::HasInstance(this))
	{
		UScopedMessageSubsystem::Get(this).HandleNetworkMessage(Packet);
	}
}
