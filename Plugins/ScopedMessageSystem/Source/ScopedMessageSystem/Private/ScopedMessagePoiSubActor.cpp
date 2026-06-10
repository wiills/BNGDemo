#include "ScopedMessagePoiSubActor.h"

AScopedMessagePoiSubActor::AScopedMessagePoiSubActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FString AScopedMessagePoiSubActor::GetPoiActorLogLabel() const
{
	return TEXT("Poi sub actor");
}
