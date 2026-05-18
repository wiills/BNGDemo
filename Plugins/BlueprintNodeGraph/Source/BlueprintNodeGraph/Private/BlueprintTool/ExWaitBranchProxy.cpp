// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintTool/ExWaitBranchProxy.h"
#include "BlueprintTool/ExLatentActionManager.h"

UExWaitBranchProxy* UExWaitBranchProxy::CreateProxy_WaitAll(UObject* WorldContextObject, FString UUID, int32 InputCount)
{
	UExWaitBranchProxy* Proxy = CreateWaitProxyCall<UExWaitBranchProxy>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExWaitBranchCompletionMode::All, 1, InputCount);
	}
	return Proxy;
}

UExWaitBranchProxy* UExWaitBranchProxy::CreateProxy_WaitAny(UObject* WorldContextObject, FString UUID, int32 InputCount)
{
	UExWaitBranchProxy* Proxy = CreateWaitProxyCall<UExWaitBranchProxy>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExWaitBranchCompletionMode::Any, 1, InputCount);
	}
	return Proxy;
}

UExWaitBranchProxy* UExWaitBranchProxy::CreateProxy_WaitCount(UObject* WorldContextObject, FString UUID, int32 InputCount,
	int32 RequiredSuccessCount)
{
	UExWaitBranchProxy* Proxy = CreateWaitProxyCall<UExWaitBranchProxy>(WorldContextObject, UUID, InputCount);
	if (Proxy && !Proxy->IsFinished() && !Proxy->IsInitialized())
	{
		Proxy->InitializeForRun(EExWaitBranchCompletionMode::Count, RequiredSuccessCount, InputCount);
	}
	return Proxy;
}

void UExWaitBranchProxy::InitializeForRun(EExWaitBranchCompletionMode InMode, int32 InRequiredSuccess, int32 ExpectedBranches)
{
	CompletionMode = InMode;
	RequiredSuccessCount = FMath::Max(1, InRequiredSuccess);
	m_InputBranchCount = ExpectedBranches;
	ReportsReceived = 0;
	SuccessReceived = 0;
	bBranchesFinished = false;
	bFinished = false;
	bInitialized = true;
}

void UExWaitBranchProxy::HandleBranchReported(bool bSuccess)
{
	if (IsFinished() || bBranchesFinished)
	{
		return;
	}

	ReportsReceived++;
	if (bSuccess)
	{
		SuccessReceived++;
	}

	bool bShouldComplete = false;
	switch (CompletionMode)
	{
	case EExWaitBranchCompletionMode::All:
		bShouldComplete = (ReportsReceived >= m_InputBranchCount);
		break;
	case EExWaitBranchCompletionMode::Any:
		bShouldComplete = (SuccessReceived >= 1);
		break;
	case EExWaitBranchCompletionMode::Count:
		bShouldComplete = (SuccessReceived >= RequiredSuccessCount);
		break;
	default:
		bShouldComplete = (ReportsReceived >= m_InputBranchCount);
		break;
	}

	if (bShouldComplete)
	{
		bBranchesFinished = true;
		OnBranchesFinished();
		if (IsFinishAfterBranches())
		{
			TryFinish();
		}
	}
}

void UExWaitBranchProxy::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExWaitBranchProxy::OnBranchesFinished] - %s"), *GetName());
}
