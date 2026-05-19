// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintTool/Proxies/ExProxy_WaitBranch.h"
#include "BlueprintTool/Proxies/ExBase_FlowProxy.h"

UExProxy_WaitBranch* UExProxy_WaitBranch::CreateProxy_WaitAll(UObject* WorldContextObject, FString UUID, int32 InputCount)
{
	UExProxy_WaitBranch* Proxy = CreateWaitProxyCall<UExProxy_WaitBranch>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExWaitBranchCompletionMode::All, 1);
	}
	return Proxy;
}

UExProxy_WaitBranch* UExProxy_WaitBranch::CreateProxy_WaitAny(UObject* WorldContextObject, FString UUID, int32 InputCount)
{
	UExProxy_WaitBranch* Proxy = CreateWaitProxyCall<UExProxy_WaitBranch>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExWaitBranchCompletionMode::Any, 1);
	}
	return Proxy;
}

UExProxy_WaitBranch* UExProxy_WaitBranch::CreateProxy_WaitCount(UObject* WorldContextObject, FString UUID, int32 InputCount,
	int32 RequiredSuccessCount)
{
	UExProxy_WaitBranch* Proxy = CreateWaitProxyCall<UExProxy_WaitBranch>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExWaitBranchCompletionMode::Count, RequiredSuccessCount);
	}
	return Proxy;
}

void UExProxy_WaitBranch::InitializeForRun(EExWaitBranchCompletionMode InMode, int32 InRequiredSuccess)
{
	CompletionMode = InMode;
	
	/** Count 模式下需要的最少成功分支数 */
	int32 RequiredSuccessCount = 1;
	switch (CompletionMode)
	{
	case EExWaitBranchCompletionMode::All:
		RequiredSuccessCount = m_ConstInputBranchCount;
		break;
	case EExWaitBranchCompletionMode::Any:
		RequiredSuccessCount = 1;
		break;
	case EExWaitBranchCompletionMode::Count:
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
