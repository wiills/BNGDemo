#pragma once

#include "CoreMinimal.h"
#include "ScopedMessagePoiActor.h"
#include "ScopedMessageTypes.h"
#include "TimerManager.h"
#include "ScopedMessagePoiRootActor.generated.h"

class APlayerController;
class UScopedMessageScopeComponent;

/**
 * Base actor for one scoped Poi instance.
 *
 * Put actors that belong to the Poi under this actor through ownership,
 * attachment, or a custom scope resolver so they share the same ScopeId.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePoiRootActor : public AScopedMessagePoiActor
{
	GENERATED_BODY()

public:
	AScopedMessagePoiRootActor();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "Scoped Message|Poi")
	FScopedMessageScopeId GetScopeId() const;

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Poi")
	void RegisterPlayer(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Poi")
	void UnregisterPlayer(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Poi")
	void RegisterAllCurrentPlayers();

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Poi")
	void UnregisterAllCurrentPlayers();

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Poi")
	void EnsurePlayerRegistrationReady();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoped Message|Poi")
	TObjectPtr<UScopedMessageScopeComponent> ScopeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message|Poi")
	bool bAutoRegisterPlayersOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message|Poi")
	bool bAutoUnregisterPlayersOnEndPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message|Poi", meta = (ClampMin = "0.01"))
	float PlayerRegistrationRetryInterval = 0.1f;

protected:
	virtual FString GetPoiActorLogLabel() const override;
	virtual void OnPoiScopeReady(FScopedMessageScopeId ScopeId) override;
	virtual void OnPlayerRegistrationReady(FScopedMessageScopeId ScopeId);

private:
	FTimerHandle PlayerRegistrationRetryTimer;
};
