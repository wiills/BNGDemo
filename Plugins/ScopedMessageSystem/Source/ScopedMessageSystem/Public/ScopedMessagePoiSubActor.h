#pragma once

#include "CoreMinimal.h"
#include "ScopedMessagePoiActor.h"
#include "ScopedMessagePoiSubActor.generated.h"

/**
 * Base actor for actors that live inside a scoped Poi.
 *
 * Kept as a semantic project-facing base class for actors that belong to a Poi.
 * It inherits scope readiness behavior from AScopedMessagePoiActor and leaves
 * business-specific subscriptions to derived classes.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePoiSubActor : public AScopedMessagePoiActor
{
	GENERATED_BODY()

public:
	AScopedMessagePoiSubActor();

protected:
	virtual FString GetPoiActorLogLabel() const override;
};
