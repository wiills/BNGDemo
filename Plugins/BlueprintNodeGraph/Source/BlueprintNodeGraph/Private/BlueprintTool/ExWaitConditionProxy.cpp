// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/ExWaitConditionProxy.h"
#include "BlueprintTool/ExLatentActionManager.h"


UExWaitConditionProxy* UExWaitConditionProxy::CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount,
	bool& Condition)
{
	const auto Proxy = CreateWaitProxyCall<UExWaitConditionProxy>(WorldContextObject, UUID, InputCount);
	Proxy->bSelfConditionValuePtr = &Condition;
	return Proxy;
}

void UExWaitConditionProxy::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExWaitConditionProxy::OnBranchesFinished] - %s"), *GetName());
	bInputOk = true;
	bEnableTick = true;
}

void UExWaitConditionProxy::Tick(float DeltaTime)
{
	if (IsDefaultSubobject())
		return;
	// check Input && Condition
	if (bInputOk && *bSelfConditionValuePtr)
	{
		bEnableTick = false;
		bInputOk = false;
		TryFinish();
	}
}

void UExWaitConditionProxy::OnFinishCall()
{
	Super::OnFinishCall();
	UE_LOG(LogAsyncAction, Display, TEXT("[UExWaitConditionProxy::OnFinishCall] - %s"), *GetName());
}
