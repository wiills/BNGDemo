// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/ExLatentTaskBase.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UExLatentTaskBase::UExLatentTaskBase(const FObjectInitializer& ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SetFlags(RF_StrongRefOnFrame);
	}
}

void UExLatentTaskBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UExLatentTaskBase, RunningState);
}

void UExLatentTaskBase::OnRep_RunningState()
{
}

UExLatentTaskBase* UExLatentTaskBase::CreateTask(UObject* WorldContextObject, TSubclassOf<UExLatentTaskBase> Class)
{
	return Cast<UExLatentTaskBase>(UGameplayStatics::SpawnObject(Class, WorldContextObject));
}

void UExLatentTaskBase::SetK2NodeInfo(const FExLatentNodeInfo K2NodeInfo)
{
	NodeInfo = K2NodeInfo;
	if (NodeInfo.TimeOut > 0)
	{
		ClearTimeoutTimer();
		GetWorld()->GetTimerManager().SetTimer(m_TimeoutTimerHandle, [Ref = TWeakObjectPtr<UExLatentTaskBase>(this)]()
		{
			if (Ref.Get() && !Ref->IsStopped())
			{
				Ref->TryStop();
			}
		}, NodeInfo.TimeOut, false);
	}
}

void UExLatentTaskBase::Activate()
{
	TryStart();
}

bool UExLatentTaskBase::IsLocal()
{
	const auto Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (!Player)
	{
		return false;
	}
	const auto PlayerRole = Player->GetLocalRole();
	if (PlayerRole == ROLE_AutonomousProxy || Player->GetNetMode() == NM_Standalone)
	{
		return true;
	}
	return false;
}

UWorld* UExLatentTaskBase::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return IsValid(GetOuter()) ? GetOuter()->GetWorld() : nullptr;
}

void UExLatentTaskBase::Terminate()
{
	UE_LOG(LogLatentTask, Display, TEXT("[EndLog][UExLatentTaskBase::Terminate] - %s, Log: %s"), *GetName(), *NodeInfo.EndLog);
	RunningState = EExLatentTaskState::Cancelled;
	MarkAsGarbage();
}

void UExLatentTaskBase::OnStart()
{
	ReceiveOnStart();
	GetStartDelegate().Broadcast();
}

void UExLatentTaskBase::OnStop()
{
	FLatentActionManager& LatentActionManager = GetWorld()->GetLatentActionManager();
	LatentActionManager.RemoveActionsForObject(this);

	ReceiveOnStop();
	GetCompleteDelegate().Broadcast();
}

void UExLatentTaskBase::ClearTimeoutTimer()
{
	if (m_TimeoutTimerHandle.IsValid() && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(m_TimeoutTimerHandle);
	}
}

void UExLatentTaskBase::BeginDestroy()
{
	ClearTimeoutTimer();
	Super::BeginDestroy();
}
