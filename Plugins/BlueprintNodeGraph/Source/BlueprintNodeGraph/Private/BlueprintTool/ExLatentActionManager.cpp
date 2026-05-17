// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExLatentActionManager.h"

void UExLatentActionProxyBase::SetK2NodeInfo(const FExLatentNodeInfo K2NodeInfo)
{
	if (m_K2NodeTimerHandle.IsValid())
	{
		return;
	}
	m_NodeInfo = K2NodeInfo;
	UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExLatentActionProxyBase::SetK2NodeInfo] - %s, Log: %s"), *GetName(), *m_NodeInfo.StartLog);
	if (m_NodeInfo.TimeOut > 0)
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExLatentActionProxyBase::SetK2NodeInfo] - %s, MaxTime: %f, Start Count Down!!!"), *GetName(), m_NodeInfo.TimeOut);
		GetWorld()->GetTimerManager().SetTimer(m_K2NodeTimerHandle, [this, WeakThis = TWeakObjectPtr<UExLatentActionProxyBase>(this)]()
		{
			if (WeakThis.IsValid() && !IsFinished())
			{
				UE_LOG(LogAsyncAction, Display, TEXT("[EndLog][UExLatentActionProxyBase::SetTimer] - %s, MaxTime: %f, TimeOut!!!, Call Finish, Log: %s"),
					*GetName(), m_NodeInfo.TimeOut, *m_NodeInfo.EndLog);
				TryFinish();
			}
		}, m_NodeInfo.TimeOut, false);
	}
}

void UExLatentActionProxyBase::TryFinish()
{
	if (IsFinished())
	{
		RemoveWaitInstance();
		return;
	}
	SetFinished(true);
	OnFinishCall();
}

void UExLatentActionProxyBase::OnFinishCall()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExLatentActionProxyBase::OnFinishCall] - %s"), *GetName());
	UE_LOG(LogAsyncAction, Display, TEXT("[EndLog][UExLatentActionProxyBase::OnFinishCall] - %s, Log: %s"), *GetName(), *m_NodeInfo.EndLog);
	SetFinished(true);
	RemoveWaitInstance();
	m_OnCompletedDelegate.Broadcast();
}

void UExLatentActionProxyBase::RemoveWaitInstance()
{
	if (!IsValid(GetWorld()) || !IsValid(GetWorld()->GetGameInstance()))
	{
		return;
	}
	SetFinished(true);
	if (IsBranchesFinished())
	{
		const auto WaitInputManager = GetWorld()->GetGameInstance()->GetSubsystem<UExLatentActionManager>();
		if (IsValid(WaitInputManager))
		{
			WaitInputManager->RemoveProxyObject(m_SelfUUID);
		}
	}
}

void UExLatentActionProxy::TryFinish()
{
	Super::TryFinish();
	if (IsFinished())
	{
		RemoveWaitInstance();
	}
}

void UExLatentActionProxy::OnFinishCall()
{
	Super::OnFinishCall();
	UE_LOG(LogAsyncAction, Display, TEXT("[UExLatentActionProxy::OnFinishCall] - %s"), *GetName());
	UE_LOG(LogAsyncAction, Display, TEXT("[EndLog][UExLatentActionProxy::OnFinishCall] - %s, Log: %s"), *GetName(), *m_NodeInfo.EndLog);
	RemoveWaitInstance();
}

void UExLatentActionProxy::RemoveWaitInstance()
{
	SetFinished(true);
	if (IsBranchesFinished())
	{
		if (!IsValid(GetWorld()) || !IsValid(GetWorld()->GetGameInstance()))
		{
			return;
		}
		const auto WaitInputManager = GetWorld()->GetGameInstance()->GetSubsystem<UExLatentActionManager>();
		if (IsValid(WaitInputManager))
		{
			WaitInputManager->RemoveProxyObject(m_SelfUUID);
		}
	}
}
