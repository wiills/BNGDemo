#pragma once

#include "CoreMinimal.h"
#include "ScopeContextProvider.h"
#include "ScopedMessageTypes.h"
#include "ScopedMessageAutomationTestTypes.generated.h"

UCLASS()
class SCOPEDMESSAGESYSTEM_API UScopedMessageAutomationScopeObject : public UObject, public IScopeContextProvider
{
	GENERATED_BODY()

public:
	virtual FScopedMessageScopeId GetScopeId_Implementation() const override
	{
		return ScopeId;
	}

	UPROPERTY()
	FScopedMessageScopeId ScopeId;
};
