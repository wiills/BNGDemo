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
 * valid ScopeId before notifying derived classes. Derived classes decide what
 * "ready" means for their role, such as registering players or subscribing to
 * local channels.
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

	/** Resolves the actor's current ScopeId through the subsystem resolver stack. */
	UFUNCTION(BlueprintPure, Category = "Scoped Message|Poi")
	FScopedMessageScopeId ResolvePoiScopeId() const;

	/** Returns true after a valid ScopeId has been cached by EnsurePoiScopeReady. */
	UFUNCTION(BlueprintPure, Category = "Scoped Message|Poi")
	bool IsPoiScopeReady() const;

	/** Blueprint hook fired once each time a new valid ScopeId is resolved. */
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
	/** Short label used by shared diagnostics. */
	virtual FString GetPoiActorLogLabel() const;

	/** C++ hook fired once each time a new valid ScopeId is resolved. */
	virtual void OnPoiScopeReady(FScopedMessageScopeId ScopeId);

private:
	FTimerHandle ScopeResolveRetryTimer;
};
