#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScopedMessageTypes.h"
#include "ScopedMessagePOIDemoRoot.generated.h"

class APlayerController;
class UScopedMessageScopeComponent;

/**
 * Demo POI root. Attach the demo terminal and door actors under this actor in
 * the World Outliner to make them share one message scope.
 */
UCLASS(BlueprintType, Blueprintable)
class SCOPEDMESSAGESYSTEM_API AScopedMessagePOIDemoRoot : public AActor
{
	GENERATED_BODY()

public:
	AScopedMessagePOIDemoRoot();

	UFUNCTION(BlueprintPure, Category = "Scoped Message Demo")
	FScopedMessageScopeId GetScopeId() const;

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Demo")
	void RegisterPlayer(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable, Category = "Scoped Message Demo")
	void UnregisterPlayer(APlayerController* PlayerController);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoped Message Demo")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoped Message Demo")
	TObjectPtr<UScopedMessageScopeComponent> ScopeComponent;
};
