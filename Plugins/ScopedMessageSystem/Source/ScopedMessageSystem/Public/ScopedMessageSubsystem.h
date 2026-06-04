#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ScopedMessageTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "ScopedMessageSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogScopedMessageSubsystem, Log, All);

/**
 * GameInstance subsystem that manages routing and broadcasting of scoped messages.
 * Uses logical scope identifiers (GameplayTags) to isolate messages within logical areas
 * (e.g. camps, level instances, or specific gameplay objects).
 */
UCLASS(DisplayName = "Scoped Message Subsystem")
class SCOPEDMESSAGESYSTEM_API UScopedMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	friend class UAsyncAction_ListenForScopedMessage;

public:
	/** Retrieves the Scoped Message Subsystem instance from a world context object. */
	static UScopedMessageSubsystem& Get(const UObject* WorldContextObject);

	/** Returns true if the Scoped Message Subsystem instance is currently valid and active. */
	static bool HasInstance(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Broadcasts a typed message struct on the specified channel.
	 *
	 * @param Channel       The message tag channel.
	 * @param Message       The payload data struct.
	 * @param ScopeContext  Optional scope boundary object; defaults to global (EmptyTag) if null or not a scope.
	 * @param Replication   Network replication rules (local, all clients, or scope-specific clients).
	 */
	template <typename FMessageStruct>
	void BroadcastMessage(
		FGameplayTag Channel,
		const FMessageStruct& Message,
		UObject* ScopeContext = nullptr,
		EScopedMessageReplication Replication = EScopedMessageReplication::LocalOnly)
	{
		const UScriptStruct* StructType = TBaseStructure<FMessageStruct>::Get();
		BroadcastMessageInternal(Channel, StructType, &Message, ResolveScopeId(ScopeContext), Replication);
	}

	/**
	 * Subscribes a lambda callback to a message channel.
	 *
	 * @param Channel       The message channel to subscribe to.
	 * @param Callback      The lambda function to trigger when messages are received.
	 * @param ScopeContext  The scope context object defining the scope boundary.
	 * @param MatchType     Exact match or partial match (including sub-categories of gameplay tags).
	 * @return A handle representing the subscription, which can be unregistered.
	 */
	template <typename FMessageStruct>
	FScopedMessageListenerHandle Subscribe(
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const FMessageStruct&)> Callback,
		UObject* ScopeContext = nullptr,
		EScopedMessageMatch MatchType = EScopedMessageMatch::ExactMatch)
	{
		auto ThunkCallback = [InnerCallback = MoveTemp(Callback)](FGameplayTag ActualTag, const UScriptStruct* SenderStructType, const void* SenderPayload)
		{
			InnerCallback(ActualTag, *reinterpret_cast<const FMessageStruct*>(SenderPayload));
		};

		const UScriptStruct* StructType = TBaseStructure<FMessageStruct>::Get();
		return SubscribeInternal(Channel, MoveTemp(ThunkCallback), StructType, ResolveScopeId(ScopeContext), MatchType, nullptr);
	}

	/**
	 * Subscribes a member function callback to a message channel with automatic weak reference safety.
	 *
	 * @param Channel       The message channel to subscribe to.
	 * @param Object        The owner object instance.
	 * @param Function      The member function pointer.
	 * @param ScopeContext  The scope context object defining the scope boundary.
	 * @param MatchType     Exact match or partial match (including sub-categories of gameplay tags).
	 * @return A handle representing the subscription.
	 */
	template <typename FMessageStruct, typename TOwner>
	FScopedMessageListenerHandle Subscribe(
		FGameplayTag Channel,
		TOwner* Object,
		void (TOwner::*Function)(FGameplayTag, const FMessageStruct&),
		UObject* ScopeContext = nullptr,
		EScopedMessageMatch MatchType = EScopedMessageMatch::ExactMatch)
	{
		TWeakObjectPtr<TOwner> WeakObject(Object);
		return Subscribe<FMessageStruct>(Channel,
			[WeakObject, Function](FGameplayTag Channel, const FMessageStruct& Payload)
			{
				if (TOwner* StrongObject = WeakObject.Get())
				{
					(StrongObject->*Function)(Channel, Payload);
				}
			},
			ScopeContext,
			MatchType);
	}

	/** Unsubscribes a listener using the provided handle. */
	void Unsubscribe(FScopedMessageListenerHandle& Handle);

	/**
	 * Blueprint exposed broadcast function using FInstancedStruct for clean wildcard input.
	 *
	 * @param Channel       The message tag channel.
	 * @param Message       The payload wildcard struct.
	 * @param ScopeContext  Optional scope boundary object.
	 * @param Replication   Replication strategy.
	 */
	UFUNCTION(BlueprintCallable, Category = "Scoped Message", DisplayName = "Broadcast Scoped Message",
		meta = (DefaultToSelf = "ScopeContext"))
	void K2_BroadcastMessage(
		FGameplayTag Channel,
		const FInstancedStruct& Message,
		UObject* ScopeContext = nullptr,
		EScopedMessageReplication Replication = EScopedMessageReplication::LocalOnly);

	/** Handles RPC distribution from the GameState replicator component on clients. */
	void HandleReplicatedMessage(
		FGameplayTag Channel,
		FName ScopeIdName,
		const UScriptStruct* PayloadType,
		const TArray<uint8>& PayloadBytes);

	/**
	 * Traverses the owner hierarchy of a given object to find a valid ScopeId.
	 *
	 * @param ScopeContext  The object to query.
	 * @return The resolved FGameplayTag.
	 */
	FGameplayTag ResolveScopeId(UObject* ScopeContext) const;

	/**
	 * Retrieves an existing dynamic ScopeId, or generates and registers one
	 * dynamically in the GameplayTagsManager if it does not already exist.
	 *
	 * @param ScopeObject   The provider object instance.
	 * @return The unique dynamic FGameplayTag.
	 */
	FGameplayTag GetOrCreateDynamicScopeId(const UObject* ScopeObject);

private:
	void BroadcastMessageInternal(
		FGameplayTag Channel,
		const UScriptStruct* PayloadType,
		const void* PayloadBytes,
		FGameplayTag ScopeId,
		EScopedMessageReplication Replication);

	FScopedMessageListenerHandle SubscribeInternal(
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
		const UScriptStruct* PayloadType,
		FGameplayTag ScopeId,
		EScopedMessageMatch MatchType,
		UObject* Owner);

	void UnsubscribeInternal(FGameplayTag ScopeId, FGameplayTag Channel, int32 HandleID);

	void CleanupInvalidListeners(FGameplayTag ScopeId, FGameplayTag Channel);

	// World and GameState observation helpers to spawn the replicator component.
	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IValues);
	void OnActorSpawned(AActor* SpawnedActor);
	void CreateReplicatorOnGameState(class AGameStateBase* GameState);

	struct FListenerList
	{
		TArray<FScopedMessageListenerData> Listeners;
		int32 NextHandleID = 1;
	};

	struct FScopeChannelMap
	{
		TMap<FGameplayTag, FListenerList> ChannelMap;
	};

	TMap<FGameplayTag, FScopeChannelMap> RoutingTable;

	TMap<TWeakObjectPtr<UObject>, FGameplayTag> DynamicScopeMap;
};
