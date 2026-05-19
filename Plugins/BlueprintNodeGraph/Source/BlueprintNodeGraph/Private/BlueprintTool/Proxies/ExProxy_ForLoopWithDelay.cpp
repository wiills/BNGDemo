// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/Proxies/ExProxy_ForLoopWithDelay.h"
#include "BlueprintTool/LatentTasks/ExLatentTaskInterface.h"


UExProxy_ForLoopWithDelay* UExProxy_ForLoopWithDelay::CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount, int32 LoopCount,
                                                            float LoopInterval, bool bNeedFistDelay)
{
	// check uuid, in case call multi-times when delay running
	const auto Proxy = CreateWaitProxyCall<UExProxy_ForLoopWithDelay>(WorldContextObject, UUID, InputCount);
	if (Proxy)
	{
		Proxy->SelfLoopCount = LoopCount;
		Proxy->SelfLoopInterval = LoopInterval;
		Proxy->bSelfNeedFistDelay = bNeedFistDelay;
	}
	return Proxy;
}

void UExProxy_ForLoopWithDelay::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_ForLoopWithDelay::OnBranchesFinished] - %s, %d, %d, bIsRunning: %d"), *GetName(), LoopCallCount, SelfLoopCount, bIsRunning);
	if (bIsRunning)
	{
		const FString Name = GetOuter() ? GetOuter()->GetName() : "";
		UE_LOG(LogAsyncAction, Error, TEXT("[UExProxy_ForLoopWithDelay::OnBranchesFinished] - Loop is Running, Called Multi-Times!!! Owner: %s"), *Name);
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
		UE_LOG(LogAsyncAction, Warning, TEXT("[UExProxy_ForLoopWithDelay::OnBranchesFinished] - %s, duplicated timer, clear timerhandle"), *GetName());
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UExProxy_ForLoopWithDelay::OnLoopCall,
		SelfLoopInterval, true, bSelfNeedFistDelay ? SelfLoopInterval : 0.f);
}

void UExProxy_ForLoopWithDelay::OnLoopCall()
{
	if (!IsValid(GetOuter()) || (Cast<IExLatentTaskInterface>(GetOuter()) && !Cast<IExLatentTaskInterface>(GetOuter())->IsRunning()))
	{
		UE_LOG(LogAsyncAction, Warning, TEXT("[UExProxy_ForLoopWithDelay::OnLoopCall] - %s, Outer Owner Not Valid or Not Running!"), *GetName());
		MakeClean();
		return;
	}
	
	LoopCallCount++;
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_ForLoopWithDelay::OnLoopCall] - %s, %d, %d"), *GetName(), LoopCallCount, SelfLoopCount);
	
	OnLoopBodyDelegate.Broadcast(LoopCallCount);
	
	// check count
	if (LoopCallCount >= SelfLoopCount)
	{
		// call finish
		TryFinish();
	}
}

void UExProxy_ForLoopWithDelay::OnFinishCall()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_ForLoopWithDelay::OnFinishCall] - %s, %d, %d"), *GetName(), LoopCallCount, SelfLoopCount);
	LoopCallCount = 0;
	bIsRunning = false;
	if(TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	RemoveWaitInstance();
	m_OnCompletedDelegate.Broadcast();
}

void UExProxy_ForLoopWithDelay::MakeClean()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_ForLoopWithDelay::MakeClean] - %s, %d, %d"), *GetName(), LoopCallCount, SelfLoopCount);
	if(TimerHandle.IsValid() && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	}
	RemoveWaitInstance();
}

void UExProxy_ForLoopWithDelay::BeginDestroy()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_ForLoopWithDelay::BeginDestroy] - %s, %d, %d"), *GetName(), LoopCallCount, SelfLoopCount);
	MakeClean();
	Super::BeginDestroy();
}
