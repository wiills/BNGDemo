#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ScopedMessageTypes.h"
#include "ScopedMessageSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogScopedMessageSubsystem, Log, All);

UCLASS(DisplayName = "Scoped Message Subsystem")
class SCOPEDMESSAGESYSTEM_API UScopedMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UScopedMessageSubsystem& Get(const UObject* WorldContextObject);
	static bool HasInstance(const UObject* WorldContextObject);

	virtual void Deinitialize() override;

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

	void Unsubscribe(FScopedMessageListenerHandle& Handle);

	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Scoped Message",
		meta = (CustomStructureParam = "Message", AllowAbstract = "false", DisplayName = "Broadcast Scoped Message"))
	void K2_BroadcastMessage(
		FGameplayTag Channel,
		const int32& Message,
		UObject* ScopeContext = nullptr,
		EScopedMessageReplication Replication = EScopedMessageReplication::LocalOnly);

	DECLARE_FUNCTION(execK2_BroadcastMessage);

	FGameplayTag ResolveScopeId(UObject* ScopeContext) const;

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

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_BroadcastMessage(
		FGameplayTag Channel,
		FGameplayTag ScopeId,
		const UScriptStruct* PayloadType,
		const TArray<uint8>& PayloadBytes);

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
};
