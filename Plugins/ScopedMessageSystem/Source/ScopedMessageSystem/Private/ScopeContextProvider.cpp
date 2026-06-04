#include "ScopeContextProvider.h"
#include "ScopedMessageSubsystem.h"

FGameplayTag IScopeContextProvider::GetScopeId() const
{
	// Cast the interface instance back to UObject to query subsystem or verify its lifetime
	if (const UObject* Object = Cast<const UObject>(this))
	{
		// Check if the subsystem exists, then get or generate a unique dynamic scope tag
		if (UScopedMessageSubsystem::HasInstance(Object))
		{
			return UScopedMessageSubsystem::Get(Object).GetOrCreateDynamicScopeId(Object);
		}
	}
	return FGameplayTag::EmptyTag;
}
