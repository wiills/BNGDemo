#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScopedMessageTypes.h"
#include "TimerManager.h"
#include "ScopedMessagePoiActor.generated.h"

/**
 * Shared base for actors that participate in a scoped Poi.
 *
 * It owns the common scene root and waits until the local process can resolve a
 * valid ScopeId before notifying derived classes.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePoiActor : public AActor
{
	GENERATED_BODY()

public:
	AScopedMessagePoiActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Poi")
	void EnsurePoiScopeReady();

	UFUNCTION(BlueprintPure, Category = "Scoped Message|Poi")
	FScopedMessageScopeId ResolvePoiScopeId() const;

	UFUNCTION(BlueprintPure, Category = "Scoped Message|Poi")
	bool IsPoiScopeReady() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Scoped Message|Poi", DisplayName = "On Poi Scope Ready")
	void BP_OnPoiScopeReady(FScopedMessageScopeId ScopeId);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoped Message|Poi")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message|Poi")
	bool bAutoResolveScopeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message|Poi", meta = (ClampMin = "0.01"))
	float ScopeResolveRetryInterval = 0.1f;

	UPROPERTY(BlueprintReadOnly, Category = "Scoped Message|Poi")
	FScopedMessageScopeId CachedPoiScopeId;

protected:
	virtual FString GetPoiActorLogLabel() const;
	virtual void OnPoiScopeReady(FScopedMessageScopeId ScopeId);

private:
	FTimerHandle ScopeResolveRetryTimer;
};
