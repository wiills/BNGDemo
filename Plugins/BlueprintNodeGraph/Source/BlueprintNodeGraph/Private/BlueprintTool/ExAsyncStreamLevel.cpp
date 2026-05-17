// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExAsyncStreamLevel.h"
#include "Engine/Engine.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UExAsyncStreamLevel* UExAsyncStreamLevel::AsyncLoadLevel(
	UObject* WorldContextObject,
	FName LevelName,
	bool bInMakeVisibleAfterLoad,
	bool bInShouldBlockOnSlowLoading)
{
	UExAsyncStreamLevel* Proxy = NewObject<UExAsyncStreamLevel>();
	Proxy->m_NodeInfo.UUID = FGuid::NewGuid().ToString();
	Proxy->m_NodeInfo.UniqueId = Proxy->m_NodeInfo.UUID;
	
	Proxy->TargetLevelName = LevelName;
	Proxy->bIsLoading = true;
	Proxy->bMakeVisibleAfterLoad = bInMakeVisibleAfterLoad;
	Proxy->bShouldBlockOnSlowLoading = bInShouldBlockOnSlowLoading;
	
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExAsyncStreamLevel* UExAsyncStreamLevel::AsyncUnloadLevel(
	UObject* WorldContextObject,
	FName LevelName)
{
	UExAsyncStreamLevel* Proxy = NewObject<UExAsyncStreamLevel>();
	Proxy->m_NodeInfo.UUID = FGuid::NewGuid().ToString();
	Proxy->m_NodeInfo.UniqueId = Proxy->m_NodeInfo.UUID;
	
	Proxy->TargetLevelName = LevelName;
	Proxy->bIsLoading = false;
	
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExAsyncStreamLevel::Activate()
{
	Super::Activate();
	
	UWorld* World = GetWorld();
	if (!World)
	{
		HandleLevelLoadFailed(TargetLevelName, TEXT("Invalid World"));
		return;
	}

	if (bIsLoading)
	{
		FLatentActionManager& LatentManager = World->GetLatentActionManager();
		FLatentActionInfo LatentInfo;
		LatentInfo.Linkage = INDEX_NONE;
		LatentInfo.UUID = GetTypeHash(FGuid::NewGuid());
		LatentInfo.ExecutionFunction = NAME_None;
		LatentInfo.CallbackTarget = this;
		if (LatentManager.FindExistingAction<FStreamLevelAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
		{
			FStreamLevelAction* NewAction = new FStreamLevelAction(true, TargetLevelName, bMakeVisibleAfterLoad, bShouldBlockOnSlowLoading, LatentInfo, World);
			LatentManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		}
	}
	else
	{
		FLatentActionManager& LatentManager = World->GetLatentActionManager();
		FLatentActionInfo LatentInfo;
		LatentInfo.Linkage = INDEX_NONE;
		LatentInfo.UUID = GetTypeHash(FGuid::NewGuid());
		LatentInfo.ExecutionFunction = NAME_None;
		LatentInfo.CallbackTarget = this;
		if (LatentManager.FindExistingAction<FStreamLevelAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
		{
			FStreamLevelAction* NewAction = new FStreamLevelAction(false, TargetLevelName, false, false, LatentInfo, World);
			LatentManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		}
	}
}

void UExAsyncStreamLevel::HandleLevelLoaded(FName LevelName)
{
	OnLevelLoaded.Broadcast(LevelName);
	SetReadyToDestroy();
}

void UExAsyncStreamLevel::HandleLevelLoadFailed(FName LevelName, const FString& ErrorMessage)
{
	OnLevelLoadFailed.Broadcast(LevelName, ErrorMessage);
	SetReadyToDestroy();
}

void UExAsyncStreamLevel::HandleLevelUnloaded(FName LevelName)
{
	OnLevelUnloaded.Broadcast(LevelName);
	SetReadyToDestroy();
}
