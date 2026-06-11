#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "AsyncAction_ListenForScopedMessage.generated.h"

class UScriptStruct;
class UScopedMessageSubsystem;
class UAsyncAction_ListenForScopedMessage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FAsyncScopedMessageDelegate,
	UAsyncAction_ListenForScopedMessage*, ProxyObject,
	FGameplayTag, ActualChannel,
	FScopedMessageScopeId, ActualScopeId
);

/**
 * Blueprint asynchronous action to listen for scoped messages on a specific channel.
 *
 * The node resolves its scope from WorldContextObject when activated and returns
 * payloads as FInstancedStruct so Blueprint graphs can inspect concrete UStruct
 * values without touching the network byte envelope.
 */
UCLASS(BlueprintType, meta = (HasDedicatedAsyncNode))
class SCOPEDMESSAGESYSTEM_API UAsyncAction_ListenForScopedMessage : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/**
	 * Asynchronously waits for a scoped message to be broadcast on the specified channel.
	 *
	 * @param WorldContextObject  The world context object (automatically supplied) and used to resolve the scope context.
	 * @param Channel             The message channel to listen for.
	 * @param PayloadType         The expected type of the message payload (optional; if left null, receives all payloads).
	 * @param MatchType           The rule used for matching the channel with broadcasted messages.
	 */
	UFUNCTION(BlueprintCallable, Category = "Scoped Message", meta = (WorldContext = "WorldContextObject", DisplayName = "Listen for Scoped Messages", BlueprintInternalUseOnly = "true"))
	static UAsyncAction_ListenForScopedMessage* ListenForScopedMessages(
		UObject* WorldContextObject,
		FGameplayTag Channel,
		UScriptStruct* PayloadType = nullptr,
		EScopedMessageMatch MatchType = EScopedMessageMatch::ExactMatch);

	/**
	 * Copies the payload received by the current delegate execution into a typed wildcard.
	 *
	 * This is primarily used by the dedicated K2 node to expose a strongly typed
	 * Payload output pin based on the selected Payload Type.
	 */
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Scoped Message", meta = (CustomStructureParam = "OutPayload"))
	bool GetPayload(UPARAM(ref) int32& OutPayload);

	DECLARE_FUNCTION(execGetPayload);

	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;

public:
	/** Called when a matching scoped message is received. */
	UPROPERTY(BlueprintAssignable)
	FAsyncScopedMessageDelegate OnMessageReceived;

private:
	/** Stores the type-erased payload for the duration of the Blueprint delegate call. */
	void HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* PayloadBytes);

private:
	const void* ReceivedMessagePayloadPtr = nullptr;

	TWeakObjectPtr<UWorld> WorldPtr;
	FGameplayTag ChannelToRegister;
	TWeakObjectPtr<UScriptStruct> MessageStructType = nullptr;
	TWeakObjectPtr<UObject> ScopeContextObject = nullptr;
	EScopedMessageMatch MessageMatchType = EScopedMessageMatch::ExactMatch;

	FScopedMessageListenerHandle ListenerHandle;
};
