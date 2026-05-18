// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/ExAsyncBlendPercent.h"
#include "BlueprintTool/ExLatentActionManager.h"


UExAsyncBlendPercentProxy* UExAsyncBlendPercentProxy::CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount
	, float& PercentSpeed1, float& PercentSpeed2)
{
	const auto Proxy = CreateWaitProxyCall<UExAsyncBlendPercentProxy>(WorldContextObject, UUID, InputCount);
	Proxy->InputPercentSpeed1 = &PercentSpeed1;
	Proxy->InputPercentSpeed2 = &PercentSpeed2;
	return Proxy;
}

void UExAsyncBlendPercentProxy::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExAsyncBlendPercentProxy::OnBranchesFinished] - %s"), *GetName());
	bInputOk = true;
	bEnableTick = true;
}

void UExAsyncBlendPercentProxy::Tick(float DeltaTime)
{
	if (IsDefaultSubobject())
		return;
	// check Input && Condition
	if (bInputOk && FinalPercent >= 1.f)
	{
		bEnableTick = false;
		bInputOk = false;
		TryFinish();
	}
}

void UExAsyncBlendPercentProxy::OnFinishCall()
{
	Super::OnFinishCall();
	UE_LOG(LogAsyncAction, Display, TEXT("[UExAsyncBlendPercentProxy::OnFinishCall] - %s"), *GetName());
}
