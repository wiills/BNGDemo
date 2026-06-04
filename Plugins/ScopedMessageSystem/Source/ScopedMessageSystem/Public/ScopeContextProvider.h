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

/**
 * Interface for objects that act as a scope boundary in the Scoped Message System.
 * Implement this interface on Actors or UObjects to define logical boundaries
 * (e.g. Dungeon Levels, Camps, Rooms) for message routing.
 */
class SCOPEDMESSAGESYSTEM_API IScopeContextProvider
{
	GENERATED_BODY()

public:
	/**
	 * Returns the scope identifier gameplay tag for this object.
	 * The default C++ implementation automatically requests a unique dynamic scope tag
	 * from the Scoped Message Subsystem.
	 *
	 * @return The GameplayTag representing this scope context.
	 */
	UFUNCTION(BlueprintCallable, Category = "Scope Context")
	virtual FGameplayTag GetScopeId() const;
};
