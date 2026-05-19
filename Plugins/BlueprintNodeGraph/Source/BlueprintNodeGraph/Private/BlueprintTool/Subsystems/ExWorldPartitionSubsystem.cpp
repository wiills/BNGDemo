// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/Subsystems/ExWorldPartitionSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/LevelStreaming.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UExWorldPartitionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UExWorldPartitionSubsystem::Deinitialize()
{
	PendingLoadDelegates.Empty();
	PendingLoadFailedDelegates.Empty();
	PendingUnloadDelegates.Empty();
	
	Super::Deinitialize();
}

UExWorldPartitionSubsystem::UExWorldPartitionSubsystem()
{
}

void UExWorldPartitionSubsystem::StreamLevel(
	UObject* WorldContextObject,
	FName LevelName,
	bool bLoadAsync,
	FOnLevelLoaded CompletionDelegate,
	FOnLevelLoadFailed FailureDelegate)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		FailureDelegate.ExecuteIfBound(LevelName, TEXT("Invalid World Context"));
		return;
	}

	if (LoadedLevels.Contains(LevelName))
	{
		CompletionDelegate.ExecuteIfBound(LevelName);
		return;
	}

	OnLevelLoadingStarted(LevelName);
	
	if (bLoadAsync)
	{
		FLatentActionManager& LatentManager = World->GetLatentActionManager();
		FLatentActionInfo LatentInfo;
		LatentInfo.Linkage = INDEX_NONE;
		LatentInfo.UUID = GetTypeHash(FGuid::NewGuid());
		LatentInfo.ExecutionFunction = NAME_None;
		LatentInfo.CallbackTarget = WorldContextObject;
		if (LatentManager.FindExistingAction<FStreamLevelAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
		{
			FStreamLevelAction* NewAction = new FStreamLevelAction(true, LevelName, true, false, LatentInfo, World);
			LatentManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		}

		if (CompletionDelegate.IsBound())
		{
			PendingLoadDelegates.FindOrAdd(LevelName).Add(CompletionDelegate);
		}
		if (FailureDelegate.IsBound())
		{
			PendingLoadFailedDelegates.FindOrAdd(LevelName).Add(FailureDelegate);
		}
	}
	else
	{
		UGameplayStatics::LoadStreamLevel(World, LevelName, true, false, FLatentActionInfo());
		OnLevelLoadingCompleted(LevelName);
		CompletionDelegate.ExecuteIfBound(LevelName);
	}
}

void UExWorldPartitionSubsystem::UnstreamLevel(
	UObject* WorldContextObject,
	FName LevelName,
	bool bUnloadAsync,
	FOnLevelUnloaded CompletionDelegate)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	if (!World)
	{
		return;
	}

	if (!LoadedLevels.Contains(LevelName))
	{
		CompletionDelegate.ExecuteIfBound(LevelName);
		return;
	}

	if (bUnloadAsync)
	{
		FLatentActionManager& LatentManager = World->GetLatentActionManager();
		FLatentActionInfo LatentInfo;
		LatentInfo.Linkage = INDEX_NONE;
		LatentInfo.UUID = GetTypeHash(FGuid::NewGuid());
		LatentInfo.ExecutionFunction = NAME_None;
		LatentInfo.CallbackTarget = WorldContextObject;
		if (LatentManager.FindExistingAction<FStreamLevelAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
		{
			FStreamLevelAction* NewAction = new FStreamLevelAction(false, LevelName, false, false, LatentInfo, World);
			LatentManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		}

		if (CompletionDelegate.IsBound())
		{
			PendingUnloadDelegates.FindOrAdd(LevelName).Add(CompletionDelegate);
		}
	}
	else
	{
		UGameplayStatics::UnloadStreamLevel(World, LevelName, FLatentActionInfo(), false);
		LoadedLevels.Remove(LevelName);
		CompletionDelegate.ExecuteIfBound(LevelName);
	}
}

bool UExWorldPartitionSubsystem::IsLevelLoaded(FName LevelName) const
{
	return LoadedLevels.Contains(LevelName);
}

TArray<FName> UExWorldPartitionSubsystem::GetLoadedLevels() const
{
	return LoadedLevels;
}

void UExWorldPartitionSubsystem::OnLevelLoadingStarted(const FName& LevelName)
{
}

void UExWorldPartitionSubsystem::OnLevelLoadingCompleted(const FName& LevelName)
{
	LoadedLevels.Add(LevelName);
	
	if (auto* Delegates = PendingLoadDelegates.Find(LevelName))
	{
		for (const auto& Delegate : *Delegates)
		{
			Delegate.ExecuteIfBound(LevelName);
		}
		PendingLoadDelegates.Remove(LevelName);
	}
}

void UExWorldPartitionSubsystem::OnLevelLoadingStatusChanged(
	const UWorldPartition* WorldPartition,
	const FWorldPartitionStreamingStatus& StreamingStatus)
{
}
