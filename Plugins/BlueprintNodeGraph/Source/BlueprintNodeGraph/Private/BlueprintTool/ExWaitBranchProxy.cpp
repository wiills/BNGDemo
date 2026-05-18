// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintTool/ExWaitBranchProxy.h"
#include "BlueprintTool/ExLatentActionManager.h"

UExWaitBranchProxy* UExWaitBranchProxy::CreateProxy_WaitAll(UObject* WorldContextObject, FString UUID, int32 InputCount)
{
	UExWaitBranchProxy* Proxy = CreateWaitProxyCall<UExWaitBranchProxy>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExWaitBranchCompletionMode::All, 1);
	}
	return Proxy;
}

UExWaitBranchProxy* UExWaitBranchProxy::CreateProxy_WaitAny(UObject* WorldContextObject, FString UUID, int32 InputCount)
{
	UExWaitBranchProxy* Proxy = CreateWaitProxyCall<UExWaitBranchProxy>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExWaitBranchCompletionMode::Any, 1);
	}
	return Proxy;
}

UExWaitBranchProxy* UExWaitBranchProxy::CreateProxy_WaitCount(UObject* WorldContextObject, FString UUID, int32 InputCount,
	int32 RequiredSuccessCount)
{
	UExWaitBranchProxy* Proxy = CreateWaitProxyCall<UExWaitBranchProxy>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExWaitBranchCompletionMode::Count, RequiredSuccessCount);
	}
	return Proxy;
}

void UExWaitBranchProxy::InitializeForRun(EExWaitBranchCompletionMode InMode, int32 InRequiredSuccess)
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

void UExWaitBranchProxy::HandleBranchReported(bool bSuccess)
{
	Super::HandleBranchReported(bSuccess);
	// remove instance
	if (IsFinished() && !IsRemoveAfterBranches())
	{
		RemoveWaitInstance();
	}
}

void UExWaitBranchProxy::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExWaitBranchProxy::OnBranchesFinished] - %s"), *GetName());
}
