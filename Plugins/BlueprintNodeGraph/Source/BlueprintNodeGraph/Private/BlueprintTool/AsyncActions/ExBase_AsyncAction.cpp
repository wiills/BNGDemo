// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/AsyncActions/ExBase_AsyncAction.h"

#include "BlueprintTool/Common/ExBlueprintDebugBubble.h"
#include "BlueprintTool/Common/ExLatentTimeoutAction.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

UExBase_AsyncAction::UExBase_AsyncAction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SetFlags(RF_StrongRefOnFrame);
	}
}

void UExBase_AsyncAction::SetK2NodeInfo(const FExLatentNodeInfo K2NodeInfo)
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
	UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExBase_AsyncAction::SetK2NodeInfo] - %s, Log: %s"), *GetName(), *m_NodeInfo.StartLog);
	if (m_NodeInfo.TimeOut > 0.f)
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExBase_AsyncAction::SetK2NodeInfo] - %s, latent timeout %f + debug bubble"), *GetName(), m_NodeInfo.TimeOut);
	}
	else
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExBase_AsyncAction::SetK2NodeInfo] - %s, debug bubble (elapsed, no timeout)"), *GetName());
	}

	ExBlueprintDebugBubble::Register(this, m_NodeInfo);

	if (m_NodeInfo.TimeOut > 0.f)
	{
		FExLatentTimeoutAction::TryRegister(this, m_NodeInfo);
	}
}

void UExBase_AsyncAction::Activate()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExBase_AsyncAction::Activate] - %s"), *GetName());
	SetFinished(false);
}

void UExBase_AsyncAction::TryFinish()
{
	ExBlueprintDebugBubble::Unregister(this, m_NodeInfo);
	if (IsFinished())
	{
		return;
	}
	SetFinished(true);
	OnFinishCall();
}

UWorld* UExBase_AsyncAction::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return IsValid(GetOuter()) ? GetOuter()->GetWorld() : nullptr;
}

void UExBase_AsyncAction::RegisterWithGameInstance(UObject* WorldContextObject)
{
	if (UWorld* FoundWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameInstance* GameInstance = FoundWorld->GetGameInstance())
		{
			RegisteredWithGameInstance = GameInstance;
		}
	}
}

void UExBase_AsyncAction::SetReadyToDestroy()
{
	ClearFlags(RF_StrongRefOnFrame);
	ClearK2NodeTimer();
}

void UExBase_AsyncAction::ClearK2NodeTimer()
{
	if (m_K2NodeTimerHandle.IsValid() && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(m_K2NodeTimerHandle);
	}
}

void UExBase_AsyncAction::BeginDestroy()
{
	ExBlueprintDebugBubble::Unregister(this, m_NodeInfo);
	ClearK2NodeTimer();
	Super::BeginDestroy();
}
