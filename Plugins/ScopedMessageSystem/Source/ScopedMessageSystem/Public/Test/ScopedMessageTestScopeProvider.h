#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "ScopeContextProvider.h"
#include "ScopedMessageTestScopeProvider.generated.h"

/**
 * A test actor that implements IScopeContextProvider to define scope boundaries in tests.
 * Users can either manually specify a ScopeId tag, or leave it blank to automatically
 * generate a unique dynamic scope tag at runtime.
 */
UCLASS(BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessageTestScopeProvider : public AActor, public IScopeContextProvider
{
	GENERATED_BODY()

public:
	AScopedMessageTestScopeProvider();

	//~ Begin IScopeContextProvider Interface
	virtual FGameplayTag GetScopeId() const override;
	//~ End IScopeContextProvider Interface

	/** Manually specified Scope ID tag. Leave empty to automatically generate a unique one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Test")
	FGameplayTag ScopeId;
};
