// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/AsyncActions/ExAsyncAction_StreamLevel.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"

UExAsyncAction_StreamLevel* UExAsyncAction_StreamLevel::AsyncLoadLevel(
	UObject* WorldContextObject,
	FName LevelName,
	bool bInMakeVisibleAfterLoad,
	bool bInShouldBlockOnSlowLoading)
{
	UExAsyncAction_StreamLevel* Proxy = NewObject<UExAsyncAction_StreamLevel>();
	Proxy->m_NodeInfo.UUID = FGuid::NewGuid().ToString();
	Proxy->m_NodeInfo.UniqueId = Proxy->m_NodeInfo.UUID;
	
	Proxy->TargetLevelName = LevelName;
	Proxy->bIsLoading = true;
	Proxy->bMakeVisibleAfterLoad = bInMakeVisibleAfterLoad;
	Proxy->bShouldBlockOnSlowLoading = bInShouldBlockOnSlowLoading;
	
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExAsyncAction_StreamLevel* UExAsyncAction_StreamLevel::AsyncUnloadLevel(
	UObject* WorldContextObject,
	FName LevelName)
{
	UExAsyncAction_StreamLevel* Proxy = NewObject<UExAsyncAction_StreamLevel>();
	Proxy->m_NodeInfo.UUID = FGuid::NewGuid().ToString();
	Proxy->m_NodeInfo.UniqueId = Proxy->m_NodeInfo.UUID;
	
	Proxy->TargetLevelName = LevelName;
	Proxy->bIsLoading = false;
	
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExAsyncAction_StreamLevel::Activate()
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

void UExAsyncAction_StreamLevel::HandleLevelLoaded(FName LevelName)
{
	OnLevelLoaded.Broadcast(LevelName);
	SetReadyToDestroy();
}

void UExAsyncAction_StreamLevel::HandleLevelLoadFailed(FName LevelName, const FString& ErrorMessage)
{
	OnLevelLoadFailed.Broadcast(LevelName, ErrorMessage);
	SetReadyToDestroy();
}

void UExAsyncAction_StreamLevel::HandleLevelUnloaded(FName LevelName)
{
	OnLevelUnloaded.Broadcast(LevelName);
	SetReadyToDestroy();
}
