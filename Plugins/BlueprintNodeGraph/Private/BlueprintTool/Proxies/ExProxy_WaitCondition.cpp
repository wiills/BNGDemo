// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/Proxies/ExProxy_WaitCondition.h"


UExProxy_WaitCondition* UExProxy_WaitCondition::CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount,
	bool& Condition)
{
	const auto Proxy = CreateWaitProxyCall<UExProxy_WaitCondition>(WorldContextObject, UUID, InputCount);
	Proxy->bSelfConditionValuePtr = &Condition;
	return Proxy;
}

void UExProxy_WaitCondition::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_WaitCondition::OnBranchesFinished] - %s"), *GetName());
	bInputOk = true;
	bEnableTick = true;
}

void UExProxy_WaitCondition::Tick(float DeltaTime)
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

void UExProxy_WaitCondition::OnFinishCall()
{
	Super::OnFinishCall();
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_WaitCondition::OnFinishCall] - %s"), *GetName());
}
