#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.h"
#include "Test/ScopedMessageTestPayload.h"
#include "ScopedMessageTestListener.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessageTestListener : public AActor
{
	GENERATED_BODY()

public:
	AScopedMessageTestListener();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Test")
	void SubscribeToChannel();

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Test")
	void UnsubscribeFromChannel();

	UFUNCTION(BlueprintImplementableEvent, Category = "Scoped Message Test", meta = (DisplayName = "On Message Received"))
	void BP_OnMessageReceived(const FString& Message, int32 Counter, FVector Location);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Test")
	FGameplayTag Channel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Test")
	UObject* ScopeContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Test")
	EScopedMessageMatch MatchType = EScopedMessageMatch::ExactMatch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Test")
	bool bAutoSubscribeOnBeginPlay = true;

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message Test")
	int32 ReceivedCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message Test")
	FString LastReceivedMessage;

private:
	FScopedMessageListenerHandle ListenerHandle;

	void OnMessageReceived(FGameplayTag InChannel, const FScopedMessageTestPayload& Payload);
};
