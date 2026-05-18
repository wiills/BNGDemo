// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExLatentActionManager.h"

#include "BlueprintTool/ExBlueprintDebugBubble.h"
#include "BlueprintTool/ExK2NodeTimeoutLatentAction.h"

void UExLatentActionProxyBase::Activate()
{
	HandleBranchReported(true);
}

void UExLatentActionProxyBase::HandleBranchReported(bool bSuccess)
{
	m_CurrentSuccessBranchCount++;
	UE_LOG(LogAsyncAction, Display, TEXT("[UExLatentActionProxyBase::HandleBranchReported] - %s, Count: %d"), *GetName(), m_CurrentSuccessBranchCount);
	OnOneBranchFinished();
	if (CheckBranchesFinished() && !IsFinished())
	{
		bBranchesFinished = true;
		OnBranchesFinished();
		if (!IsCustomFinish())
		{
			TryFinish();
		}
	}
}

void UExLatentActionProxyBase::SetK2NodeInfo(const FExLatentNodeInfo K2NodeInfo)
{
	if (ExBlueprintDebugBubble::HasActiveRegistration(this, K2NodeInfo))
	{
		return;
	}
	if (FExK2NodeTimeoutLatentAction::HasExistingForProxy(this, K2NodeInfo.UUID))
	{
		return;
	}
	m_NodeInfo = K2NodeInfo;
	UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExLatentActionProxyBase::SetK2NodeInfo] - %s, Log: %s"), *GetName(), *m_NodeInfo.StartLog);

	ExBlueprintDebugBubble::Register(this, m_NodeInfo);

	if (m_NodeInfo.TimeOut > 0.f)
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExLatentActionProxyBase::SetK2NodeInfo] - %s, latent timeout %f + debug bubble"), *GetName(), m_NodeInfo.TimeOut);
		FExK2NodeTimeoutLatentAction::TryRegister(this, m_NodeInfo);
	}
	else
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExLatentActionProxyBase::SetK2NodeInfo] - %s, debug bubble (elapsed, no timeout)"), *GetName());
	}
}

void UExLatentActionProxyBase::TryFinish()
{
	ExBlueprintDebugBubble::Unregister(this, m_NodeInfo);
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
	UE_LOG(LogAsyncAction, Display, TEXT("[EndLog][UExLatentActionProxyBase::OnFinishCall] - %s, Log: %s"), *GetName(), *m_NodeInfo.EndLog);
	SetFinished(true);
	if (IsRemoveAfterBranches())
	{
		RemoveWaitInstance();
	}
	m_OnCompletedDelegate.Broadcast();
}

void UExLatentActionProxyBase::RemoveWaitInstance()
{
	if (!IsValid(GetWorld()) || !IsValid(GetWorld()->GetGameInstance()))
	{
		return;
	}
	if (!IsRemoveAfterBranches())
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

void UExLatentActionProxyBase::BeginDestroy()
{
	ExBlueprintDebugBubble::Unregister(this, m_NodeInfo);
	Super::BeginDestroy();
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
