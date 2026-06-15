#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ScopedMessageSubsystem.generated.h"

class AGameStateBase;
class APlayerController;
class FStructOnScope;
class UScopedMessageClientBridgeComponent;
class UScopedMessageReplicatorComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogScopedMessageSubsystem, Log, All);

/**
 * Optional project resolver hook.
 *
 * Return true and write OutScopeId to override the default owner/attachment/outer
 * traversal. Return false to let later resolvers or the default resolver try.
 */
DECLARE_DELEGATE_RetVal_TwoParams(bool, FScopedMessageScopeResolver, UObject* /*ScopeContext*/, FScopedMessageScopeId& /*OutScopeId*/);

/**
 * 作用域占用变化事件。
 * Fired whenever the count of players registered as interested in a scope changes,
 * including transitions to and from zero. This is a networking-driven signal, but it
 * doubles as a reliable wake/sleep source for upper layers (e.g. ScopedLogicGraph)
 * without exposing the internal interest table. Note: only meaningful on authority.
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnScopeOccupancyChanged, FScopedMessageScopeId /*ScopeId*/, int32 /*PlayerCount*/);

/**
 * GameInstance subsystem that routes messages by logical scope and channel.
 *
 * Intended use: one replicated scope ID per Poi/mission-space instance, with
 * static GameplayTag channels inside that scope.
 */
UCLASS(DisplayName = "Scoped Message Subsystem")
class SCOPEDMESSAGESYSTEM_API UScopedMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend class UAsyncAction_ListenForScopedMessage;

public:
	/** Returns the subsystem for a valid world context, asserting if the context is invalid. */
	static UScopedMessageSubsystem& Get(const UObject* WorldContextObject);

	/** Returns the subsystem for a valid world context, or nullptr if one is not available. */
	static UScopedMessageSubsystem* GetInstance(const UObject* WorldContextObject);

	/** Returns whether a scoped message subsystem is available for this context. */
	static bool HasInstance(const UObject* WorldContextObject);

	/** Human-readable net mode prefix used by diagnostics and demo logs. */
	static FString GetNetModePrefix(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	template <typename FMessageStruct>
	void BroadcastMessage(
		const UObject* WorldContextObject,
		FGameplayTag Channel,
		const FMessageStruct& Payload,
		EScopedMessageReplication Replication = EScopedMessageReplication::LocalOnly)
	{
		// Scope is resolved at broadcast time so the same actor class can be reused
		// under many Poi roots without carrying per-instance channel tags.
		const UScriptStruct* StructType = FMessageStruct::StaticStruct();
		BroadcastMessageInternal(Channel, StructType, &Payload, ResolveScopeId(const_cast<UObject*>(WorldContextObject)), Replication);
	}

	template <typename FMessageStruct>
	static bool BroadcastMessageIfAvailable(
		const UObject* WorldContextObject,
		FGameplayTag Channel,
		const FMessageStruct& Payload,
		EScopedMessageReplication Replication = EScopedMessageReplication::LocalOnly)
	{
		if (UScopedMessageSubsystem* Subsystem = GetInstance(WorldContextObject))
		{
			Subsystem->BroadcastMessage(WorldContextObject, Channel, Payload, Replication);
			return true;
		}

		return false;
	}

	template <typename FMessageStruct>
	FScopedMessageListenerHandle Subscribe(
		UObject* Object,
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const FMessageStruct&)> Callback,
		EScopedMessageMatch MatchType = EScopedMessageMatch::ExactMatch)
	{
		// Store listeners through a type-erased thunk; the registered payload type is
		// checked before this callback is invoked.
		auto ThunkCallback = [InnerCallback = MoveTemp(Callback)](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
		{
			InnerCallback(ActualTag, *reinterpret_cast<const FMessageStruct*>(SenderPayload));
		};

		const UScriptStruct* StructType = FMessageStruct::StaticStruct();
		return SubscribeInternal(Channel, MoveTemp(ThunkCallback), StructType, ResolveScopeId(Object), MatchType, Object);
	}

	template <typename FMessageStruct, typename TOwner>
	FScopedMessageListenerHandle Subscribe(
		FGameplayTag Channel,
		TOwner* Object,
		void (TOwner::*Function)(FGameplayTag, const FMessageStruct&),
		EScopedMessageMatch MatchType = EScopedMessageMatch::ExactMatch)
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		return Subscribe<FMessageStruct>(Object, Channel,
			[WeakObject, Function](FGameplayTag ActualChannel, const FMessageStruct& Payload)
			{
				if (TOwner* StrongObject = WeakObject.Get())
				{
					(StrongObject->*Function)(ActualChannel, Payload);
				}
			},
			MatchType);
	}

	/** Removes a listener represented by Handle and invalidates the handle. */
	void Unsubscribe(FScopedMessageListenerHandle& Handle);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message", DisplayName = "Broadcast Scoped Message",
		meta = (WorldContext = "WorldContextObject"))
	static void K2_BroadcastMessage(
		const UObject* WorldContextObject,
		FGameplayTag Channel,
		const FInstancedStruct& Payload,
		EScopedMessageReplication Replication = EScopedMessageReplication::LocalOnly);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Interest", meta = (WorldContext = "WorldContextObject"))
	static void K2_RegisterPlayerForScope(
		const UObject* WorldContextObject,
		APlayerController* PlayerController,
		FScopedMessageScopeId ScopeId);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Interest", meta = (WorldContext = "WorldContextObject"))
	static void K2_UnregisterPlayerForScope(
		const UObject* WorldContextObject,
		APlayerController* PlayerController,
		FScopedMessageScopeId ScopeId);

	/** Registers a PlayerController as interested in a ScopeId for scoped client delivery and client-to-server validation. */
	void RegisterPlayerForScope(APlayerController* PlayerController, FScopedMessageScopeId ScopeId);

	/** Removes a PlayerController from a ScopeId interest set. */
	void UnregisterPlayerForScope(APlayerController* PlayerController, FScopedMessageScopeId ScopeId);

	/** Current number of players registered as interested in ScopeId. */
	int32 GetScopePlayerCount(FScopedMessageScopeId ScopeId) const;

	/** Subscribe here to react to scope occupancy transitions (wake/sleep upper-layer logic). */
	FOnScopeOccupancyChanged OnScopeOccupancyChanged;

	/** Adds a project-specific resolver that runs before the default traversal resolver. */
	FDelegateHandle RegisterScopeResolver(FScopedMessageScopeResolver Resolver);

	/** Removes a resolver previously returned by RegisterScopeResolver. */
	void UnregisterScopeResolver(FDelegateHandle ResolverHandle);

	/** Handles a trusted network packet that has already arrived in this process. */
	void HandleNetworkMessage(const FScopedMessageNetworkPacket& Packet);

	/** Handles a client-originated packet after validating that Sender is registered for the ScopeId. */
	void HandleClientMessage(APlayerController* Sender, const FScopedMessageNetworkPacket& Packet);

	/** Runs custom resolvers first, then falls back to ResolveScopeIdDefault. */
	FScopedMessageScopeId ResolveScopeId(UObject* ScopeContext) const;

	/** Default resolver: direct provider, components, owner chain, attachment chain, then outer chain. */
	FScopedMessageScopeId ResolveScopeIdDefault(UObject* ScopeContext) const;

	/** Utility for purely local scopes that are tied to a UObject lifetime. */
	FScopedMessageScopeId GetOrCreateLocalScopeId(const UObject* ScopeObject);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Debug")
	void DumpRoutingTable() const;

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Debug")
	void DumpScopeResolution(UObject* ScopeContext) const;

private:
	/** Shared broadcast implementation for C++, Blueprint, and deserialized network packets. */
	void BroadcastMessageInternal(
		FGameplayTag Channel,
		const UScriptStruct* PayloadType,
		const void* PayloadBytes,
		FScopedMessageScopeId ScopeId,
		EScopedMessageReplication Replication);

	FScopedMessageListenerHandle SubscribeInternal(
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
		const UScriptStruct* PayloadType,
		FScopedMessageScopeId ScopeId,
		EScopedMessageMatch MatchType,
		UObject* Owner);

	void DispatchMessage(
		FScopedMessageScopeId ScopeId,
		FGameplayTag Channel,
		const UScriptStruct* PayloadType,
		const void* PayloadBytes);

	void UnsubscribeInternal(FScopedMessageScopeId ScopeId, FGameplayTag Channel, int32 HandleID);

	/** Removes stale weak listeners for one route and collapses empty maps. */
	void CleanupInvalidListeners(FScopedMessageScopeId ScopeId, FGameplayTag Channel);

	/** Removes all listeners owned by an actor that is ending play. */
	void CleanupInvalidListenersForOwner(const UObject* Owner);

	/** Global weak-listener sweep used during world cleanup. */
	void CleanupAllInvalidListeners();

	/** Removes invalid PlayerController weak references from interest sets. */
	void CleanupInvalidPlayerInterests();

	/** Broadcasts the current player count for a scope to occupancy listeners. */
	void NotifyScopeOccupancyChanged(FScopedMessageScopeId ScopeId);

	bool BuildNetworkPacket(
		FScopedMessageScopeId ScopeId,
		FGameplayTag Channel,
		const UScriptStruct* PayloadType,
		const void* PayloadBytes,
		FScopedMessageNetworkPacket& OutPacket) const;

	const UScriptStruct* ResolvePayloadType(const FScopedMessageNetworkPacket& Packet) const;
	bool DeserializePacket(const FScopedMessageNetworkPacket& Packet, FStructOnScope& OutMessage) const;

	void SendPacketToServer(const FScopedMessageNetworkPacket& Packet);
	void SendPacketToAllClients(const FScopedMessageNetworkPacket& Packet);
	void SendPacketToScopedClients(const FScopedMessageNetworkPacket& Packet);

	bool IsAuthorityOrStandalone() const;
	bool IsClientNetMode() const;
	bool IsPlayerRegisteredForScope(APlayerController* PlayerController, FScopedMessageScopeId ScopeId) const;

	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues);
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);
	void OnActorSpawned(AActor* SpawnedActor);
	UFUNCTION()
	void OnActorEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
	void CreateReplicatorOnGameState(AGameStateBase* GameState);
	void CreateClientBridgeOnPlayerController(APlayerController* PlayerController);

	UScopedMessageClientBridgeComponent* FindClientBridgeForPlayer(APlayerController* PlayerController) const;
	UScopedMessageReplicatorComponent* FindReplicator() const;

	struct FListenerList
	{
		TArray<FScopedMessageListenerData> Listeners;
		int32 NextHandleID = 1;
	};

	struct FScopeChannelMap
	{
		TMap<FGameplayTag, FListenerList> ChannelMap;
	};

	struct FRegisteredScopeResolver
	{
		FDelegateHandle Handle;
		FScopedMessageScopeResolver Resolver;
	};

	/** ScopeId -> Channel -> listeners. This is the isolation boundary of the plugin. */
	TMap<FScopedMessageScopeId, FScopeChannelMap> RoutingTable;

	/** UObject-lifetime local scopes generated by GetOrCreateLocalScopeId. */
	TMap<TWeakObjectPtr<UObject>, FScopedMessageScopeId> LocalScopeMap;

	/** Server-side player interest table used by ClientToServer and ServerToScopedClients. */
	TMap<FScopedMessageScopeId, TArray<TWeakObjectPtr<APlayerController>>> ScopePlayerInterests;

	/** Custom resolvers are evaluated in registration order before the default resolver. */
	TArray<FRegisteredScopeResolver> CustomScopeResolvers;

	/** Per-world spawn delegate handles so bridge components can be attached to late-spawned actors. */
	TMap<TWeakObjectPtr<UWorld>, FDelegateHandle> ActorSpawnedDelegateHandles;

	/** Actors whose EndPlay delegate is bound for eager listener cleanup. */
	TSet<TWeakObjectPtr<AActor>> EndPlayBoundActors;
};
