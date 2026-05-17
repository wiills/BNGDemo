// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/ExAsyncActionBase.h"
#include "ExLatentSpawnProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExLatentSpawnCompleteDelegate, const AActor*, Actor); //-V1100

/**
 * Deferred Spawn Character Node
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExLatentSpawnProxy : public UExAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, meta=(DisplayName="OnSpawning"))
	FOnExLatentSpawnCompleteDelegate SpawningDelegate;
	UPROPERTY(BlueprintAssignable, meta=(DisplayName="OnSpawnFinish"))
	FOnExLatentSpawnCompleteDelegate SpawnFinishDelegate;

	/** Spawn new Actor on the network authority (server) */
	UFUNCTION(BlueprintCallable, Category = "LatentTasks", meta = (WorldContext = "WorldContextObject", DisplayName="Latent Spawn Actor By Class", BlueprintInternalUseOnly = "TRUE"))
	static UExLatentSpawnProxy* LatentSpawnActor(UObject* WorldContextObject, TSubclassOf<AActor> Class, FVector SpawnLocation, FRotator SpawnRotation);

	virtual void Activate() override;

private:
	UPROPERTY()
	AActor* SpawnedActor;
	UPROPERTY()
	TSubclassOf<AActor> _ActorClass;
	UPROPERTY()
	FVector _Location;
	UPROPERTY()
	FRotator _Rotation;
};
