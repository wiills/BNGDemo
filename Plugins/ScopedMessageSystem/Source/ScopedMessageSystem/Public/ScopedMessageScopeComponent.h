#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScopeContextProvider.h"
#include "ScopedMessageTypes.h"
#include "ScopedMessageScopeComponent.generated.h"

/**
 * Replicated scope provider for Poi/root actors.
 *
 * Add this component to the actor that represents one Poi instance. Child actors can
 * resolve this ScopeId through owner/attachment/outer traversal and route messages
 * without leaking into another Poi using the same channels.
 */
UCLASS(ClassGroup = (ScopedMessage), meta = (BlueprintSpawnableComponent))
class SCOPEDMESSAGESYSTEM_API UScopedMessageScopeComponent : public UActorComponent, public IScopeContextProvider
{
	GENERATED_BODY()

public:
	UScopedMessageScopeComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual FScopedMessageScopeId GetScopeId_Implementation() const override;

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Scope")
	void SetScopeId(FScopedMessageScopeId InScopeId);

	UFUNCTION(BlueprintPure, Category = "Scoped Message|Scope")
	FScopedMessageScopeId GetConfiguredScopeId() const { return ScopeId; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Scoped Message|Scope")
	FScopedMessageScopeId ScopeId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoped Message|Scope")
	bool bGenerateScopeIdOnAuthority = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoped Message|Scope")
	FName GeneratedScopePrefix = TEXT("Poi");
};
