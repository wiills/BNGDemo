#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScopedMessageTypes.h"
#include "TimerManager.h"
#include "ScopedMessagePoiSubActor.generated.h"

/**
 * Base actor for actors that live inside a scoped Poi.
 *
 * It waits until the local world can resolve a valid ScopeId, which avoids
 * client-side BeginPlay subscribing into the empty global scope before the Poi
 * root's replicated ScopeId arrives.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePoiSubActor : public AActor
{
	GENERATED_BODY()

public:
	AScopedMessagePoiSubActor();

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
	virtual void OnPoiScopeReady(FScopedMessageScopeId ScopeId);

private:
	FTimerHandle ScopeResolveRetryTimer;
};
