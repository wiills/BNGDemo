#pragma once

#include "CoreMinimal.h"
#include "ScopedMessagePoiRootActor.h"
#include "ScopedMessagePoiDemoRoot.generated.h"

/**
 * Demo Poi root. Attach the demo terminal and door actors under this actor in
 * the World Outliner to make them share one message scope.
 */
UCLASS(BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePoiDemoRoot : public AScopedMessagePoiRootActor
{
	GENERATED_BODY()

public:
	AScopedMessagePoiDemoRoot();
};
