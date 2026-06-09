#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.h"
#include "ScopedMessageReplicatorComponent.generated.h"

/**
 * Replicated ActorComponent attached to GameState to replicate scoped messages across the network.
 * Since GameState is always relevant (bAlwaysRelevant = true), this component inherits that property,
 * guaranteeing that its NetMulticast RPCs will reach all connected clients.
 */
UCLASS(ClassGroup = (ScopedMessage), meta = (BlueprintSpawnableComponent))
class SCOPEDMESSAGESYSTEM_API UScopedMessageReplicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScopedMessageReplicatorComponent();

	/**
	 * Multicast RPC to replicate a message to all clients.
	 */
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_BroadcastMessage(const FScopedMessageNetworkPacket& Packet);
};
