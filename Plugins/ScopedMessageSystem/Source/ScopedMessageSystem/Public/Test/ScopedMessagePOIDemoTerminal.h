#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.h"
#include "Test/ScopedMessagePOIDemoTypes.h"
#include "ScopedMessagePOIDemoTerminal.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePOIDemoTerminal : public AActor
{
	GENERATED_BODY()

public:
	AScopedMessagePOIDemoTerminal();

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Demo")
	void ActivateTerminal();

	UFUNCTION(Server, Reliable)
	void Server_ActivateTerminal();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoped Message Demo")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FName TerminalId = TEXT("TerminalA");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FGameplayTag ActivationChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	EScopedMessageReplication Replication = EScopedMessageReplication::ServerToScopedClients;

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message Demo")
	int32 ActivationCount = 0;
};
