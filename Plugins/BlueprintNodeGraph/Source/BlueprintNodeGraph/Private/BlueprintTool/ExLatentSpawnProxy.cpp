// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/ExLatentSpawnProxy.h"
#include "BlueprintTool/ExLatentProxyDefine.h"
#include "BlueprintTool/ExLatentTaskInterface.h"


UExLatentSpawnProxy* UExLatentSpawnProxy::LatentSpawnActor(UObject* WorldContextObject, TSubclassOf<AActor> Class,
														FVector SpawnLocation, FRotator SpawnRotation)
{
	UExLatentSpawnProxy* Proxy = NewObject<UExLatentSpawnProxy>(WorldContextObject);
	Proxy->_ActorClass = Class;
	Proxy->_Location = SpawnLocation;
	Proxy->_Rotation = SpawnRotation;
	return Proxy;
}

void UExLatentSpawnProxy::Activate()
{
	Super::Activate();

	UE_LOG(LogLatentTask, Display, TEXT("[UExLatentSpawnProxy::Activate] - %s"), *GetName());
	if (!GetWorld())
	{
		UE_LOG(LogLatentTask, Warning, TEXT("[UExLatentSpawnProxy::Activate] WorldContext Null"));
		return;
	}
	if (!_ActorClass.Get())
	{
		UE_LOG(LogLatentTask, Warning, TEXT("[UExLatentSpawnProxy::Activate] No Actor Class"));
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
