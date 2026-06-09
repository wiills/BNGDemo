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

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//~ Begin IScopeContextProvider Interface
	virtual FScopedMessageScopeId GetScopeId_Implementation() const override;
	//~ End IScopeContextProvider Interface

	/** Manually specified Scope ID tag. Leave empty to automatically generate a unique one. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Scoped Message Test")
	FScopedMessageScopeId ScopeId;
};
