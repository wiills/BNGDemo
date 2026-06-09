#pragma once

#include "CoreMinimal.h"
#include "Engine/CancellableAsyncAction.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.h"
#include "AsyncAction_ListenForScopedMessage.generated.h"

class UScriptStruct;
class UScopedMessageSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FAsyncScopedMessageDelegate,
	FGameplayTag, ActualChannel,
	const FScopedMessagePayload&, Payload,
	FScopedMessageScopeId, ActualScopeId
);

/**
 * Blueprint asynchronous action to listen for scoped messages on a specific channel.
 */
UCLASS(BlueprintType, meta = (HasDedicatedAsyncNode = "false"))
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
	UFUNCTION(BlueprintCallable, Category = "Scoped Message", meta = (WorldContext = "WorldContextObject", DisplayName = "Listen for Scoped Messages"))
	static UAsyncAction_ListenForScopedMessage* ListenForScopedMessages(
		UObject* WorldContextObject,
		FGameplayTag Channel,
		UScriptStruct* PayloadType = nullptr,
		EScopedMessageMatch MatchType = EScopedMessageMatch::ExactMatch);

	virtual void Activate() override;
	virtual void SetReadyToDestroy() override;

public:
	/** Called when a matching scoped message is received. */
	UPROPERTY(BlueprintAssignable)
	FAsyncScopedMessageDelegate OnMessageReceived;

private:
	void HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* PayloadBytes);

private:
	TWeakObjectPtr<UWorld> WorldPtr;
	FGameplayTag ChannelToRegister;
	TWeakObjectPtr<UScriptStruct> MessageStructType = nullptr;
	TWeakObjectPtr<UObject> ScopeContextObject = nullptr;
	EScopedMessageMatch MessageMatchType = EScopedMessageMatch::ExactMatch;

	FScopedMessageListenerHandle ListenerHandle;
};
