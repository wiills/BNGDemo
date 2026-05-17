// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExAsyncActionBase.h"

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
	if (m_K2NodeTimerHandle.IsValid())
	{
		return;
	}
	m_NodeInfo = K2NodeInfo;
	UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExAsyncActionBase::SetK2NodeInfo] - %s, Log: %s"), *GetName(), *m_NodeInfo.StartLog);
	if (m_NodeInfo.TimeOut > 0)
	{
		UE_LOG(LogAsyncAction, Display, TEXT("[StartLog][UExAsyncActionBase::SetK2NodeInfo] - %s, MaxTime: %f, Start Count Down!!!"), *GetName(), m_NodeInfo.TimeOut);
		GetWorld()->GetTimerManager().SetTimer(m_K2NodeTimerHandle, [Ref = TWeakObjectPtr<UExAsyncActionBase>(this)]()
		{
			if (Ref.Get() && !Ref->IsFinished())
			{
				UE_LOG(LogAsyncAction, Display, TEXT("[EndLog][UExAsyncActionBase::SetTimer] - %s, MaxTime: %f, TimeOut!!!, Call Finish, Log: %s"),
					*Ref->GetName(), Ref->m_NodeInfo.TimeOut, *Ref->m_NodeInfo.EndLog);
				Ref->TryFinish();
			}
		}, m_NodeInfo.TimeOut, false);
	}
}

void UExAsyncActionBase::Activate()
{
	UE_LOG(LogAsyncAction, Display, TEXT("[UExAsyncActionBase::Activate] - %s"), *GetName());
	SetFinished(false);
}

void UExAsyncActionBase::TryFinish()
{
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
		if (FoundWorld->GetGameInstance())
		{
			RegisterWithGameInstance(FoundWorld->GetGameInstance());
		}
	}
}

void UExAsyncActionBase::RegisterWithGameInstance(UGameInstance* GameInstance)
{
	if (GameInstance)
	{
		RegisteredWithGameInstance = GameInstance;
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
	ClearK2NodeTimer();
	Super::BeginDestroy();
}
