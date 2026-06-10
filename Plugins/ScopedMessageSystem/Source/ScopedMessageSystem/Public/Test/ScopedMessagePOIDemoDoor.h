#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ScopedMessagePoiSubActor.h"
#include "ScopedMessageTypes.h"
#include "Test/ScopedMessagePoiDemoTypes.h"
#include "ScopedMessagePoiDemoDoor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePoiDemoDoor : public AScopedMessagePoiSubActor
{
	GENERATED_BODY()

public:
	AScopedMessagePoiDemoDoor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Demo")
	void SubscribeToTerminal();

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Demo")
	void UnsubscribeFromTerminal();

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Demo")
	void ResetDoor();

	UFUNCTION(BlueprintImplementableEvent, Category = "Scoped Message Demo")
	void BP_OnDoorOpened(FName TriggeredByTerminalId, int32 ActivationCount);

	UFUNCTION()
	void OnRep_IsOpen();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FName DoorId = TEXT("DoorA");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FName RequiredTerminalId = TEXT("TerminalA");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FGameplayTag ActivationChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	bool bAutoSubscribeOnBeginPlay = true;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IsOpen, Category = "Scoped Message Demo")
	bool bIsOpen = false;

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message Demo")
	int32 OpenCount = 0;

protected:
	virtual void OnPoiScopeReady(FScopedMessageScopeId ScopeId) override;

private:
	FScopedMessageListenerHandle ListenerHandle;
	FScopedMessageScopeId SubscribedScopeId;

	void OnTerminalActivated(FGameplayTag Channel, const FScopedMessageDemoTerminalActivatedPayload& Payload);
	void OpenDoor(const FScopedMessageDemoTerminalActivatedPayload& Payload);
};
