#include "Test/ScopedMessageTestScopeProvider.h"

AScopedMessageTestScopeProvider::AScopedMessageTestScopeProvider()
{
	PrimaryActorTick.bCanEverTick = false;

	// Establish a default root component.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

FGameplayTag AScopedMessageTestScopeProvider::GetScopeId() const
{
	// Return the manually configured tag if it is valid.
	if (ScopeId.IsValid())
	{
		return ScopeId;
	}
	// Fall back to the default interface implementation to auto-generate a unique tag.
	return IScopeContextProvider::GetScopeId();
}
