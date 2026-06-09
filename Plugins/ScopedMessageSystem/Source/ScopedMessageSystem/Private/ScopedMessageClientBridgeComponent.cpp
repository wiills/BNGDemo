#include "ScopedMessageClientBridgeComponent.h"

#include "GameFramework/PlayerController.h"
#include "ScopedMessageSubsystem.h"

UScopedMessageClientBridgeComponent::UScopedMessageClientBridgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UScopedMessageClientBridgeComponent::Server_SendScopedMessage_Implementation(const FScopedMessageNetworkPacket& Packet)
{
	APlayerController* Sender = Cast<APlayerController>(GetOwner());
	if (Sender && UScopedMessageSubsystem::HasInstance(this))
	{
		UScopedMessageSubsystem::Get(this).HandleClientMessage(Sender, Packet);
	}
}

void UScopedMessageClientBridgeComponent::Client_ReceiveScopedMessage_Implementation(const FScopedMessageNetworkPacket& Packet)
{
	if (UScopedMessageSubsystem::HasInstance(this))
	{
		UScopedMessageSubsystem::Get(this).HandleNetworkMessage(Packet);
	}
}
