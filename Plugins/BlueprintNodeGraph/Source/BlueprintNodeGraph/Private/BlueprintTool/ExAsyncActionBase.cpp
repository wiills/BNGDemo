// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExAsyncActionBase.h"

#include "BlueprintTool/ExBlueprintDebugBubble.h"
#include "BlueprintTool/ExK2NodeTimeoutLatentAction.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

UExAsyncActionBase::UExAsyncActionBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SetFlags(RF_StrongRefOnFrame);
	}
}

void UExAsyncActionBase::SetK2NodeInfo(const FExLatentNodeInfo K2NodeInfo)
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
	UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExAsyncActionBase::SetK2NodeInfo] - %s, Log: %s"), *GetName(), *m_NodeInfo.StartLog);
	if (m_NodeInfo.TimeOut > 0.f)
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExAsyncActionBase::SetK2NodeInfo] - %s, latent timeout %f + debug bubble"), *GetName(), m_NodeInfo.TimeOut);
	}
	else
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExAsyncActionBase::SetK2NodeInfo] - %s, debug bubble (elapsed, no timeout)"), *GetName());
	}

	ExBlueprintDebugBubble::Register(this, m_NodeInfo);

	if (m_NodeInfo.TimeOut > 0.f)
	{
		FExK2NodeTimeoutLatentAction::TryRegister(this, m_NodeInfo);
	}
}

void UExAsyncActionBase::Activate()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExAsyncActionBase::Activate] - %s"), *GetName());
	SetFinished(false);
}

void UExAsyncActionBase::TryFinish()
{
	ExBlueprintDebugBubble::Unregister(this, m_NodeInfo);
	if (IsFinished())
	{
		return;
	}
	SetFinished(true);
	OnFinishCall();
}

UWorld* UExAsyncActionBase::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return IsValid(GetOuter()) ? GetOuter()->GetWorld() : nullptr;
}

void UExAsyncActionBase::RegisterWithGameInstance(UObject* WorldContextObject)
{
	if (UWorld* FoundWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameInstance* GameInstance = FoundWorld->GetGameInstance())
		{
			RegisteredWithGameInstance = GameInstance;
		}
	}
}

void UExAsyncActionBase::SetReadyToDestroy()
{
	ClearFlags(RF_StrongRefOnFrame);
	ClearK2NodeTimer();
}

void UExAsyncActionBase::ClearK2NodeTimer()
{
	if (m_K2NodeTimerHandle.IsValid() && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(m_K2NodeTimerHandle);
	}
}

void UExAsyncActionBase::BeginDestroy()
{
	ExBlueprintDebugBubble::Unregister(this, m_NodeInfo);
	ClearK2NodeTimer();
	Super::BeginDestroy();
}
