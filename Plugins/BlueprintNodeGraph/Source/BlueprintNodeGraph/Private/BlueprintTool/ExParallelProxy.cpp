// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExParallelProxy.h"
#include "BlueprintTool/ExLatentActionManager.h"

UExParallelProxy* UExParallelProxy::CreateProxy_All(UObject* WorldContextObject, int32 BranchCount)
{
	auto* Proxy = NewObject<UExParallelProxy>();
	Proxy->ExpectedBranchCount = BranchCount;
	Proxy->Mode = EExParallelMode::All;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExParallelProxy* UExParallelProxy::CreateProxy_Any(UObject* WorldContextObject, int32 BranchCount)
{
	auto* Proxy = NewObject<UExParallelProxy>();
	Proxy->ExpectedBranchCount = BranchCount;
	Proxy->Mode = EExParallelMode::Any;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExParallelProxy* UExParallelProxy::CreateProxy_Count(UObject* WorldContextObject, int32 BranchCount, int32 RequiredCount)
{
	auto* Proxy = NewObject<UExParallelProxy>();
	Proxy->ExpectedBranchCount = BranchCount;
	Proxy->Mode = EExParallelMode::Count;
	Proxy->SuccessfulBranchCount = RequiredCount;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExParallelProxy::RegisterBranch(FString BranchUUID)
{
	BranchUUIDs.Add(BranchUUID);
	BranchResults.Add(BranchUUID, false);
}

void UExParallelProxy::BranchCompleted(FString BranchUUID, bool bSuccess)
{
	if (!BranchResults.Contains(BranchUUID))
	{
		return;
	}

	BranchResults[BranchUUID] = bSuccess;
	CompletedBranchCount++;

	OnBranchCompleted.Broadcast();
	CheckCompletion();
}

void UExParallelProxy::Activate()
{
	Super::Activate();
}

void UExParallelProxy::CheckCompletion()
{
	if (IsFinished())
	{
		return;
	}

	bool bShouldComplete = false;

	switch (Mode)
	{
	case EExParallelMode::All:
		bShouldComplete = (CompletedBranchCount >= ExpectedBranchCount);
		break;

	case EExParallelMode::Any:
		for (const auto& Result : BranchResults)
		{
			if (Result.Value)
			{
				bShouldComplete = true;
				break;
			}
		}
		break;

	case EExParallelMode::Count:
	{
		int32 SuccessCount = 0;
		for (const auto& Result : BranchResults)
		{
			if (Result.Value)
			{
				SuccessCount++;
			}
		}
		bShouldComplete = (SuccessCount >= SuccessfulBranchCount);
		break;
	}
	}

	if (bShouldComplete)
	{
		OnCompleted.Broadcast();
		SetReadyToDestroy();
	}
}
