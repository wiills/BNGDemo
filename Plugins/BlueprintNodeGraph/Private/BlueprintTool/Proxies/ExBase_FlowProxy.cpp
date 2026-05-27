// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/Proxies/ExBase_FlowProxy.h"

#include "BlueprintTool/Common/ExBlueprintDebugBubble.h"
#include "BlueprintTool/Subsystems/ExLatentActionManager.h"
#include "BlueprintTool/Common/ExLatentTimeoutAction.h"

void UExBase_FlowProxy::Activate()
{
	HandleBranchReported(true);
}

void UExBase_FlowProxy::HandleBranchReported(bool bSuccess)
{
	m_CurrentSuccessBranchCount++;
	UE_LOG(LogAsyncAction, Display, TEXT("[UExBase_FlowProxy::HandleBranchReported] - %s, Count: %d"), *GetName(), m_CurrentSuccessBranchCount);
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

void UExBase_FlowProxy::SetK2NodeInfo(const FExLatentNodeInfo K2NodeInfo)
{
	if (ExBlueprintDebugBubble::HasActiveRegistration(this, K2NodeInfo))
	{
		return;
	}
	if (FExLatentTimeoutAction::HasExistingForProxy(this, K2NodeInfo.UUID))
	{
		return;
	}
	m_NodeInfo = K2NodeInfo;
	UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExBase_FlowProxy::SetK2NodeInfo] - %s, Log: %s"), *GetName(), *m_NodeInfo.StartLog);

	ExBlueprintDebugBubble::Register(this, m_NodeInfo);

	if (m_NodeInfo.TimeOut > 0.f)
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExBase_FlowProxy::SetK2NodeInfo] - %s, latent timeout %f + debug bubble"), *GetName(), m_NodeInfo.TimeOut);
		FExLatentTimeoutAction::TryRegister(this, m_NodeInfo);
	}
	else
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExBase_FlowProxy::SetK2NodeInfo] - %s, debug bubble (elapsed, no timeout)"), *GetName());
	}
}

void UExBase_FlowProxy::TryFinish()
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

void UExBase_FlowProxy::OnFinishCall()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[EndLog][UExBase_FlowProxy::OnFinishCall] - %s, Log: %s"), *GetName(), *m_NodeInfo.EndLog);
	SetFinished(true);
	if (IsRemoveAfterBranches())
	{
		RemoveWaitInstance();
	}
	m_OnCompletedDelegate.Broadcast();
}

void UExBase_FlowProxy::RemoveWaitInstance()
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

void UExBase_FlowProxy::BeginDestroy()
{
	ExBlueprintDebugBubble::Unregister(this, m_NodeInfo);
	Super::BeginDestroy();
}

void UExAsyncAction_BranchSync::TryFinish()
{
	Super::TryFinish();
	if (IsFinished())
	{
		RemoveWaitInstance();
	}
}

void UExAsyncAction_BranchSync::OnFinishCall()
{
	Super::OnFinishCall();
	UE_LOG(LogAsyncAction, Display, TEXT("[UExAsyncAction_BranchSync::OnFinishCall] - %s"), *GetName());
	UE_LOG(LogAsyncAction, Display, TEXT("[EndLog][UExAsyncAction_BranchSync::OnFinishCall] - %s, Log: %s"), *GetName(), *m_NodeInfo.EndLog);
	RemoveWaitInstance();
}

void UExAsyncAction_BranchSync::RemoveWaitInstance()
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
