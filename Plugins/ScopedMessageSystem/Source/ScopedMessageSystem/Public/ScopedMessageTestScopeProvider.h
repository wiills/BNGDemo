#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScopeContextProvider.h"
#include "GameplayTagContainer.h"
#include "ScopedMessageTestScopeProvider.generated.h"

UCLASS(BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessageTestScopeProvider : public AActor, public IScopeContextProvider
{
	GENERATED_BODY()

public:
	AScopedMessageTestScopeProvider();

	virtual FGameplayTag GetScopeId() const override { return ScopeId; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message Test")
	FGameplayTag ScopeId;
};
