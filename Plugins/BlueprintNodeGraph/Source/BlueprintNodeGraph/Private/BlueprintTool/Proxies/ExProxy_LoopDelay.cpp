// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintTool/Proxies/ExProxy_LoopDelay.h"


UExProxy_LoopDelay* UExProxy_LoopDelay::CreateProxy(UObject* WorldContextObject, FString UUID, float Duration, bool bNeedFirstDelay)
{
	const auto WaitInputManager = WorldContextObject->GetWorld()->GetGameInstance()->GetSubsystem<UExLatentActionManager>();
	if (!WaitInputManager)
		return nullptr;
	auto ObjectUUID = FString::Format(TEXT("{0}_{1}"),
		{ FStringFormatArg(GetTypeHash(WorldContextObject)), FStringFormatArg(UUID) });
	auto Proxy = WaitInputManager->GetProxyObject<UExProxy_LoopDelay>(ObjectUUID);
	if (!Proxy)
	{
		Proxy = NewObject<UExProxy_LoopDelay>(WorldContextObject);
		Proxy->SetUUIDAndCount(ObjectUUID, 1);
		Proxy->SelfDuration = Duration;
		WaitInputManager->SetProxyObject(ObjectUUID, Proxy);
		Proxy->CallCount = bNeedFirstDelay ? 0 : -1;
		Proxy->DelayOverCount = Proxy->CallCount;
	}
	return Proxy;
}

void UExProxy_LoopDelay::OnBranchesFinished()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_LoopDelay::OnBranchesFinished] - %s"), *GetName());
	if (SelfDuration>0 && CallCount>0)
	{
		FTimerHandle TempTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TempTimerHandle, this, &UExProxy_LoopDelay::OnTimeOver, SelfDuration*CallCount, false);
	}else
	{
		// Using this will introduce at least 1 frame delay
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UExProxy_LoopDelay::OnTimeOver);
	}
}

void UExProxy_LoopDelay::OnTimeOver()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExProxy_LoopDelay::OnTimeOver] - %s"), *GetName());
	DelayOverCount++;
	m_OnCompletedDelegate.Broadcast();
	// reset
	if (DelayOverCount >= CallCount)
	{
		CallCount = 0;
		DelayOverCount = 0;
		RemoveWaitInstance();
	}
}
