#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScopedMessageTypes.h"
#include "ScopedMessageClientBridgeComponent.generated.h"

/**
 * Per-PlayerController network bridge used for owner-targeted scoped messages.
 *
 * GameState multicast is still used for all-client messages. Scoped client delivery
 * goes through PlayerController-owned bridge components so the server can send only
 * to players registered as interested in the target ScopeId.
 */
UCLASS(ClassGroup = (ScopedMessage))
class SCOPEDMESSAGESYSTEM_API UScopedMessageClientBridgeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScopedMessageClientBridgeComponent();

	UFUNCTION(Server, Reliable)
	void Server_SendScopedMessage(const FScopedMessageNetworkPacket& Packet);

	UFUNCTION(Client, Reliable)
	void Client_ReceiveScopedMessage(const FScopedMessageNetworkPacket& Packet);
};
