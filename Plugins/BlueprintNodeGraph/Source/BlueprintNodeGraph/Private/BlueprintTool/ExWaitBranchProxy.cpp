// Fill out your copyright notice in the Description page of Project Settings.

#include "BlueprintTool/ExWaitBranchProxy.h"
#include "BlueprintTool/ExLatentActionManager.h"

UExWaitBranchProxy* UExWaitBranchProxy::CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount,
	EExParallelMode InCompletionMode, int32 InRequiredSuccessCount)
{
	UExWaitBranchProxy* Proxy = CreateWaitProxyCall<UExWaitBranchProxy>(WorldContextObject, UUID, InputCount);
	if (Proxy)
	{
		Proxy->InitializeForRun(InCompletionMode, InRequiredSuccessCount, InputCount);
	}
	return Proxy;
}

void UExWaitBranchProxy::InitializeForRun(EExParallelMode InMode, int32 InRequiredSuccess, int32 ExpectedBranches)
{
	CompletionMode = InMode;
	RequiredSuccessCount = FMath::Max(1, InRequiredSuccess);
	m_InputBranchCount = ExpectedBranches;
	ReportsReceived = 0;
	SuccessReceived = 0;
	bBranchesFinished = false;
	bFinished = false;
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
	case EExParallelMode::All:
		bShouldComplete = (ReportsReceived >= m_InputBranchCount);
		break;
	case EExParallelMode::Any:
		bShouldComplete = (SuccessReceived >= 1);
		break;
	case EExParallelMode::Count:
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
