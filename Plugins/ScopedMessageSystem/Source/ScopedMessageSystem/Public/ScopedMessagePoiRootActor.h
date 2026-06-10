#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScopedMessageTypes.h"
#include "ScopedMessagePoiRootActor.generated.h"

class APlayerController;
class UScopedMessageScopeComponent;

/**
 * Base actor for one scoped Poi instance.
 *
 * Put actors that belong to the Poi under this actor through ownership,
 * attachment, or a custom scope resolver so they share the same ScopeId.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePoiRootActor : public AActor
{
	GENERATED_BODY()

public:
	AScopedMessagePoiRootActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "Scoped Message|Poi")
	FScopedMessageScopeId GetScopeId() const;

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Poi")
	void RegisterPlayer(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Poi")
	void UnregisterPlayer(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Poi")
	void RegisterAllCurrentPlayers();

	UFUNCTION(BlueprintCallable, Category = "Scoped Message|Poi")
	void UnregisterAllCurrentPlayers();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoped Message|Poi")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoped Message|Poi")
	TObjectPtr<UScopedMessageScopeComponent> ScopeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message|Poi")
	bool bAutoRegisterPlayersOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoped Message|Poi")
	bool bAutoUnregisterPlayersOnEndPlay = true;
};
