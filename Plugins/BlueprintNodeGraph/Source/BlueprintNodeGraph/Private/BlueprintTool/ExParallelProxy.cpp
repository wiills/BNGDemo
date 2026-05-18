// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExParallelProxy.h"

UExParallelProxy* UExParallelProxy::CreateProxy(UObject* WorldContextObject, int32 InBranchCount, EExParallelMode InMode, int32 InRequiredCount)
{
	auto* Proxy = NewObject<UExParallelProxy>();
	Proxy->ExpectedBranchCount = InBranchCount;
	Proxy->Mode = InMode;
	Proxy->SuccessfulBranchCount = InRequiredCount;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExParallelProxy::RegisterBranch(FString BranchUUID)
{
	if (IsFinished())
	{
		return;
	}
	BranchUUIDs.Add(BranchUUID);
	BranchResults.Add(BranchUUID, false);
}

void UExParallelProxy::BranchCompleted(FString BranchUUID, bool bSuccess)
{
	if (IsFinished())
	{
		return;
	}
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
