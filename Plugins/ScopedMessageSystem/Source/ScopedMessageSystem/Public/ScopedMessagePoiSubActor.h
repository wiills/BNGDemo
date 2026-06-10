#pragma once

#include "CoreMinimal.h"
#include "ScopedMessagePoiActor.h"
#include "ScopedMessagePoiSubActor.generated.h"

/**
 * Base actor for actors that live inside a scoped Poi.
 *
 * Kept as a semantic project-facing base class for actors that belong to a Poi.
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
