// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintTool/Proxies/ExProxy_WaitBranch.h"

UExProxy_WaitBranch* UExProxy_WaitBranch::CreateProxy_WaitAll(UObject* WorldContextObject, FString UUID, int32 InputCount)
{
	UExProxy_WaitBranch* Proxy = CreateWaitProxyCall<UExProxy_WaitBranch>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExBranchMode::All, 1);
	}
	return Proxy;
}

UExProxy_WaitBranch* UExProxy_WaitBranch::CreateProxy_WaitAny(UObject* WorldContextObject, FString UUID, int32 InputCount)
{
	UExProxy_WaitBranch* Proxy = CreateWaitProxyCall<UExProxy_WaitBranch>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExBranchMode::Any, 1);
	}
	return Proxy;
}

UExProxy_WaitBranch* UExProxy_WaitBranch::CreateProxy_WaitCount(UObject* WorldContextObject, FString UUID, int32 InputCount,
	int32 RequiredSuccessCount)
{
	UExProxy_WaitBranch* Proxy = CreateWaitProxyCall<UExProxy_WaitBranch>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExBranchMode::Count, RequiredSuccessCount);
	}
	return Proxy;
}

void UExProxy_WaitBranch::InitializeForRun(EExBranchMode InMode, int32 InRequiredSuccess)
{
	BranchMode = InMode;
	
	/** Count 模式下需要的最少成功分支数 */
	int32 RequiredSuccessCount = 1;
	switch (BranchMode)
	{
	case EExBranchMode::All:
		RequiredSuccessCount = m_ConstInputBranchCount;
		break;
	case EExBranchMode::Any:
		RequiredSuccessCount = 1;
		break;
	case EExBranchMode::Count:
		RequiredSuccessCount = InRequiredSuccess;
		break;
	default:
		RequiredSuccessCount = 1;
		break;
	}
	SetNeedBranchCount(RequiredSuccessCount);
	
	bBranchesFinished = false;
	bFinished = false;
	bInitialized = true;
}

void UExProxy_WaitBranch::OnOneBranchFinished()
{
	// remove instance
	if (IsFinished() && !IsRemoveAfterBranches())
	{
		RemoveWaitInstance();
	}
}

void UExProxy_WaitBranch::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_WaitBranch::OnBranchesFinished] - %s"), *GetName());
}
