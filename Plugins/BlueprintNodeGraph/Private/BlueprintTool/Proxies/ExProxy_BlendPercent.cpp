// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/Proxies/ExProxy_BlendPercent.h"


UExProxy_BlendPercent* UExProxy_BlendPercent::CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount
	, float& PercentSpeed1, float& PercentSpeed2)
{
	const auto Proxy = CreateWaitProxyCall<UExProxy_BlendPercent>(WorldContextObject, UUID, InputCount);
	Proxy->InputPercentSpeed1 = &PercentSpeed1;
	Proxy->InputPercentSpeed2 = &PercentSpeed2;
	return Proxy;
}

void UExProxy_BlendPercent::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_BlendPercent::OnBranchesFinished] - %s"), *GetName());
	bInputOk = true;
	bEnableTick = true;
}

void UExProxy_BlendPercent::Tick(float DeltaTime)
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

void UExProxy_BlendPercent::OnFinishCall()
{
	Super::OnFinishCall();
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_BlendPercent::OnFinishCall] - %s"), *GetName());
}
