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

DECLARE_DELEGATE_RetVal_TwoParams(bool, FScopedMessageScopeResolver, UObject* /*ScopeContext*/, FScopedMessageScopeId& /*OutScopeId*/);

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
	static UScopedMessageSubsystem& Get(const UObject* WorldContextObject);
	static bool HasInstance(const UObject* WorldContextObject);
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
		const UScriptStruct* StructType = FMessageStruct::StaticStruct();
		BroadcastMessageInternal(Channel, StructType, &Payload, ResolveScopeId(const_cast<UObject*>(WorldContextObject)), Replication);
	}

	template <typename FMessageStruct>
	FScopedMessageListenerHandle Subscribe(
		UObject* Object,
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const FMessageStruct&)> Callback,
		EScopedMessageMatch MatchType = EScopedMessageMatch::ExactMatch)
	{
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

	void RegisterPlayerForScope(APlayerController* PlayerController, FScopedMessageScopeId ScopeId);
	void UnregisterPlayerForScope(APlayerController* PlayerController, FScopedMessageScopeId ScopeId);

	FDelegateHandle RegisterScopeResolver(FScopedMessageScopeResolver Resolver);
	void UnregisterScopeResolver(FDelegateHandle ResolverHandle);

	void HandleNetworkMessage(const FScopedMessageNetworkPacket& Packet);
	void HandleClientMessage(APlayerController* Sender, const FScopedMessageNetworkPacket& Packet);

	FScopedMessageScopeId ResolveScopeId(UObject* ScopeContext) const;
	FScopedMessageScopeId ResolveScopeIdDefault(UObject* ScopeContext) const;
	FScopedMessageScopeId GetOrCreateLocalScopeId(const UObject* ScopeObject);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Debug")
	void DumpRoutingTable() const;

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Debug")
	void DumpScopeResolution(UObject* ScopeContext) const;

private:
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
	void CleanupInvalidListeners(FScopedMessageScopeId ScopeId, FGameplayTag Channel);
	void CleanupInvalidListenersForOwner(const UObject* Owner);
	void CleanupAllInvalidListeners();
	void CleanupInvalidPlayerInterests();

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

	TMap<FScopedMessageScopeId, FScopeChannelMap> RoutingTable;
	TMap<TWeakObjectPtr<UObject>, FScopedMessageScopeId> LocalScopeMap;
	TMap<FScopedMessageScopeId, TArray<TWeakObjectPtr<APlayerController>>> ScopePlayerInterests;
	TArray<FRegisteredScopeResolver> CustomScopeResolvers;
	TMap<TWeakObjectPtr<UWorld>, FDelegateHandle> ActorSpawnedDelegateHandles;
	TSet<TWeakObjectPtr<AActor>> EndPlayBoundActors;
};
