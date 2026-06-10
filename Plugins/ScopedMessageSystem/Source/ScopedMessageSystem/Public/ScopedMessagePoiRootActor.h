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
 * Root actors also provide the optional player-interest helpers used by
 * ServerToScopedClients and ClientToServer validation.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePoiRootActor : public AScopedMessagePoiActor
{
	GENERATED_BODY()

public:
	AScopedMessagePoiRootActor();

	virtual void BeginPlay() override;
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

	/** When true, the root overwrites any authored ScopeId with a fresh server-generated runtime ScopeId. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message|Poi")
	bool bGenerateRuntimeScopeIdOnAuthority = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message|Poi", meta = (ClampMin = "0.01"))
	float PlayerRegistrationRetryInterval = 0.1f;

protected:
	virtual FString GetPoiActorLogLabel() const override;

	/** When the root scope is ready, optional auto-registration waits for players and then registers them. */
	virtual void OnPoiScopeReady(FScopedMessageScopeId ScopeId) override;

	virtual void OnPlayerRegistrationReady(FScopedMessageScopeId ScopeId);

private:
	FTimerHandle PlayerRegistrationRetryTimer;
};
