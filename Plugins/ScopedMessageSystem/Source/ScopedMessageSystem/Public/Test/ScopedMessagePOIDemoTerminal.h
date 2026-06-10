#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ScopedMessagePoiSubActor.h"
#include "ScopedMessageTypes.h"
#include "Test/ScopedMessagePoiDemoTypes.h"
#include "ScopedMessagePoiDemoTerminal.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePoiDemoTerminal : public AScopedMessagePoiSubActor
{
	GENERATED_BODY()

public:
	AScopedMessagePoiDemoTerminal();

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Demo")
	void ActivateTerminal();

	UFUNCTION(Server, Reliable)
	void Server_ActivateTerminal();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FName TerminalId = TEXT("TerminalA");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	FGameplayTag ActivationChannel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Demo")
	EScopedMessageReplication Replication = EScopedMessageReplication::ServerToScopedClients;

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message Demo")
	int32 ActivationCount = 0;
};
