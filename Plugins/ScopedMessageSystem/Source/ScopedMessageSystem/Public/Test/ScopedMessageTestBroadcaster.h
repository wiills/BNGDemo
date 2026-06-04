#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTypes.h"
#include "Test/ScopedMessageTestPayload.h"
#include "ScopedMessageTestBroadcaster.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessageTestBroadcaster : public AActor
{
	GENERATED_BODY()

public:
	AScopedMessageTestBroadcaster();

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Test")
	void BroadcastTestMessage();

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Test")
	void StartAutoBroadcast(float Interval);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Test")
	void StopAutoBroadcast();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Test")
	FGameplayTag Channel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Test")
	FString MessageText = TEXT("Hello from Broadcaster");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Test")
	UObject* ScopeContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Test")
	EScopedMessageReplication Replication = EScopedMessageReplication::LocalOnly;

private:
	FTimerHandle AutoBroadcastTimer;
	int32 BroadcastCounter = 0;

	void OnAutoBroadcast();
};
