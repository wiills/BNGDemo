// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/AsyncActions/ExAsyncAction_SpawnActor.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"

UExAsyncAction_SpawnActor* UExAsyncAction_SpawnActor::LatentSpawnActor(UObject* WorldContextObject, TSubclassOf<AActor> Class,
														FVector SpawnLocation, FRotator SpawnRotation)
{
	UExAsyncAction_SpawnActor* Proxy = NewObject<UExAsyncAction_SpawnActor>(WorldContextObject);
	Proxy->_ActorClass = Class;
	Proxy->_Location = SpawnLocation;
	Proxy->_Rotation = SpawnRotation;
	return Proxy;
}

void UExAsyncAction_SpawnActor::Activate()
{
	Super::Activate();

	UE_LOG(LogLatentTask, Display, TEXT("[UExAsyncAction_SpawnActor::Activate] - %s"), *GetName());
	if (!GetWorld())
	{
		UE_LOG(LogLatentTask, Warning, TEXT("[UExAsyncAction_SpawnActor::Activate] WorldContext Null"));
		return;
	}
	if (!_ActorClass.Get())
	{
		UE_LOG(LogLatentTask, Warning, TEXT("[UExAsyncAction_SpawnActor::Activate] No Actor Class"));
		return;
	}
	
	const FTransform SpawnTransform(_Rotation, _Location);	
	SpawnedActor = GetWorld()->SpawnActorDeferred<AActor>(_ActorClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	// on spawning
	SpawningDelegate.Broadcast(SpawnedActor);
	
	SpawnedActor->FinishSpawning(SpawnTransform);
	
	// on spawn finished
	SpawnFinishDelegate.Broadcast(SpawnedActor);
}
