#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
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
	 * Multicast RPC to replicate and distribute the serialized message payload to all clients.
	 *
	 * @param Channel       The message tag channel.
	 * @param ScopeIdName   The name of the ScopeId tag to reconstruct on the client.
	 * @param PayloadType   The reflection structure type of the message.
	 * @param PayloadBytes  The serialized struct payload bytes.
	 */
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_BroadcastMessage(
		FGameplayTag Channel,
		FName ScopeIdName,
		const UScriptStruct* PayloadType,
		const TArray<uint8>& PayloadBytes);
};
