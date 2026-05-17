// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/ExForLoopWithDelayProxy.h"
#include "BlueprintTool/ExLatentActionManager.h"
#include "BlueprintTool/ExLatentTaskInterface.h"


UExForLoopWithDelayProxy* UExForLoopWithDelayProxy::CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount, int32 LoopCount,
                                                            float LoopInterval, bool bNeedFistDelay)
{
	// check uuid, in case call multi-times when delay running
	const auto Proxy = CreateWaitProxyCall<UExForLoopWithDelayProxy>(WorldContextObject, UUID, InputCount);
	if (Proxy)
	{
		Proxy->SelfLoopCount = LoopCount;
		Proxy->SelfLoopInterval = LoopInterval;
		Proxy->bSelfNeedFistDelay = bNeedFistDelay;
	}
	return Proxy;
}

void UExForLoopWithDelayProxy::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExForLoopWithDelayProxy::OnBranchesFinished] - %s, %d, %d, bIsRunning: %d"), *GetName(), LoopCallCount, SelfLoopCount, bIsRunning);
	if (bIsRunning)
	{
		const FString Name = GetOuter() ? GetOuter()->GetName() : "";
		UE_LOG(LogAsyncAction, Error, TEXT("[UExForLoopWithDelayProxy::OnBranchesFinished] - Loop is Running, Called Multi-Times!!! Owner: %s"), *Name);
		return;
	}

	// running flag
	bIsRunning = true;
	
	// no count, call over
	if (SelfLoopCount <= 0)
	{
		TryFinish();
		return;
	}
	
	// interval<=0, just for call
	if (SelfLoopInterval <= 0)
	{
		for (int32 i=0; i<SelfLoopCount; i++)
		{
			OnLoopCall();
		}
		return;
	}
	
	/*
	 * call with delay
	 * call once first, if no need of FirstDelay
	 */
	if(TimerHandle.IsValid())
	{
		UE_LOG(LogAsyncAction, Warning, TEXT("[UExForLoopWithDelayProxy::OnBranchesFinished] - %s, duplicated timer, clear timerhandle"), *GetName());
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UExForLoopWithDelayProxy::OnLoopCall,
		SelfLoopInterval, true, bSelfNeedFistDelay ? SelfLoopInterval : 0.f);
}

void UExForLoopWithDelayProxy::OnLoopCall()
{
	if (!IsValid(GetOuter()) || (Cast<IExLatentTaskInterface>(GetOuter()) && !Cast<IExLatentTaskInterface>(GetOuter())->IsRunning()))
	{
		UE_LOG(LogAsyncAction, Warning, TEXT("[UExForLoopWithDelayProxy::OnLoopCall] - %s, Outer Owner Not Valid or Not Running!"), *GetName());
		MakeClean();
		return;
	}
	
	LoopCallCount++;
	UE_LOG(LogAsyncAction, Display, TEXT("[UExForLoopWithDelayProxy::OnLoopCall] - %s, %d, %d"), *GetName(), LoopCallCount, SelfLoopCount);
	
	OnLoopBodyDelegate.Broadcast(LoopCallCount);
	
	// check count
	if (LoopCallCount >= SelfLoopCount)
	{
		// call finish
		TryFinish();
	}
}

void UExForLoopWithDelayProxy::OnFinishCall()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExForLoopWithDelayProxy::OnFinishCall] - %s, %d, %d"), *GetName(), LoopCallCount, SelfLoopCount);
	LoopCallCount = 0;
	bIsRunning = false;
	if(TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	RemoveWaitInstance();
	m_OnCompletedDelegate.Broadcast();
}

void UExForLoopWithDelayProxy::MakeClean()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExForLoopWithDelayProxy::MakeClean] - %s, %d, %d"), *GetName(), LoopCallCount, SelfLoopCount);
	if(TimerHandle.IsValid() && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	RemoveWaitInstance();
}

void UExForLoopWithDelayProxy::BeginDestroy()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExForLoopWithDelayProxy::BeginDestroy] - %s, %d, %d"), *GetName(), LoopCallCount, SelfLoopCount);
	MakeClean();
	Super::BeginDestroy();
}
