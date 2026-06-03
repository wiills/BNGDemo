#include "ScopedMessageTestScopeProvider.h"

AScopedMessageTestScopeProvider::AScopedMessageTestScopeProvider()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}
