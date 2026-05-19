// Copyright Epic Games, Inc. All Rights Reserved.

#include "BlueprintTool/LatentTasks/ExBase_LatentTask.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

UExBase_LatentTask::UExBase_LatentTask(const FObjectInitializer& ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SetFlags(RF_StrongRefOnFrame);
	}
}

void UExBase_LatentTask::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UExBase_LatentTask, RunningState);
}

void UExBase_LatentTask::OnRep_RunningState()
{
}

UExBase_LatentTask* UExBase_LatentTask::CreateTask(UObject* WorldContextObject, TSubclassOf<UExBase_LatentTask> Class)
{
	return Cast<UExBase_LatentTask>(UGameplayStatics::SpawnObject(Class, WorldContextObject));
}

void UExBase_LatentTask::SetK2NodeInfo(const FExLatentNodeInfo K2NodeInfo)
{
	NodeInfo = K2NodeInfo;
	if (NodeInfo.TimeOut > 0)
	{
		ClearTimeoutTimer();
		GetWorld()->GetTimerManager().SetTimer(m_TimeoutTimerHandle, [Ref = TWeakObjectPtr<UExBase_LatentTask>(this)]()
		{
			if (Ref.Get() && !Ref->IsStopped())
			{
				Ref->TryStop();
			}
		}, NodeInfo.TimeOut, false);
	}
}

void UExBase_LatentTask::Activate()
{
	TryStart();
}

bool UExBase_LatentTask::IsLocal()
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

UWorld* UExBase_LatentTask::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}
	return IsValid(GetOuter()) ? GetOuter()->GetWorld() : nullptr;
}

void UExBase_LatentTask::Terminate()
{
	UE_LOG(LogLatentTask, Display, TEXT("[EndLog][UExBase_LatentTask::Terminate] - %s, Log: %s"), *GetName(), *NodeInfo.EndLog);
	RunningState = EExLatentTaskState::Cancelled;
	MarkAsGarbage();
}

void UExBase_LatentTask::OnStart()
{
	ReceiveOnStart();
	GetStartDelegate().Broadcast();
}

void UExBase_LatentTask::OnStop()
{
	FLatentActionManager& LatentActionManager = GetWorld()->GetLatentActionManager();
	LatentActionManager.RemoveActionsForObject(this);

	ReceiveOnStop();
	GetCompleteDelegate().Broadcast();
}

void UExBase_LatentTask::ClearTimeoutTimer()
{
	if (m_TimeoutTimerHandle.IsValid() && GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(m_TimeoutTimerHandle);
	}
}

void UExBase_LatentTask::BeginDestroy()
{
	ClearTimeoutTimer();
	Super::BeginDestroy();
}
