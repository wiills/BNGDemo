#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.h"
#include "ScopedMessageReplicatorComponent.generated.h"

/**
 * GameState-owned component for all-client scoped message replication.
 *
 * It is created by the subsystem on authority when needed. Since GameState is
 * normally relevant to every client, the multicast path is appropriate for
 * ServerToAllClients. Scoped client delivery uses PlayerController bridges
 * instead.
 */
UCLASS(ClassGroup = (ScopedMessage), meta = (BlueprintSpawnableComponent))
class SCOPEDMESSAGESYSTEM_API UScopedMessageReplicatorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScopedMessageReplicatorComponent();

	/** Multicast RPC used by EScopedMessageReplication::ServerToAllClients. */
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_BroadcastMessage(const FScopedMessageNetworkPacket& Packet);
};
