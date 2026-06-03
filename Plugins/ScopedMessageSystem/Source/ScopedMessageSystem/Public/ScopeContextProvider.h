#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "ScopeContextProvider.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UScopeContextProvider : public UInterface
{
	GENERATED_BODY()
};

class SCOPEDMESSAGESYSTEM_API IScopeContextProvider
{
	GENERATED_BODY()

public:
	virtual FGameplayTag GetScopeId() const = 0;
};
