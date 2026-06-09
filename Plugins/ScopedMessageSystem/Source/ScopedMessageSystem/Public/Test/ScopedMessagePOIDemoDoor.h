#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.h"
#include "Test/ScopedMessagePOIDemoTypes.h"
#include "ScopedMessagePOIDemoDoor.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePOIDemoDoor : public AActor
{
	GENERATED_BODY()

public:
	AScopedMessagePOIDemoDoor();

	virtual void BeginPlay() override;
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoped Message Demo")
	TObjectPtr<USceneComponent> SceneRoot;

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

private:
	FScopedMessageListenerHandle ListenerHandle;

	void OnTerminalActivated(FGameplayTag Channel, const FScopedMessageDemoTerminalActivatedPayload& Payload);
	void OpenDoor(const FScopedMessageDemoTerminalActivatedPayload& Payload);
};
