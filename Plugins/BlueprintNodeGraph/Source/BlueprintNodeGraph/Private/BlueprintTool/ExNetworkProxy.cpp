// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/ExNetworkProxy.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

void UExReplicationProxy::Activate()
{
	Super::Activate();
	
	UWorld* World = GetWorld();
	if (!World)
	{
		SetReadyToDestroy();
		return;
	}

	if (ReplicationMode == EExReplicationMode::Multicast)
	{
		OnReplicated.Broadcast();
		UE_LOG(LogAsyncAction, Display, TEXT("[UExReplicationProxy] Multicast replication for state: %s"), *FlowState.StateName);
	}
	else if (ReplicationMode == EExReplicationMode::ServerOnly)
	{
		if (World->GetNetMode() == NM_Client)
		{
			SetReadyToDestroy();
			return;
		}
		OnReplicated.Broadcast();
		UE_LOG(LogAsyncAction, Display, TEXT("[UExReplicationProxy] Server-only replication for state: %s"), *FlowState.StateName);
	}

	SetReadyToDestroy();
}

UExReplicationProxy* UExReplicationProxy::CreateProxy_Replicate(UObject* WorldContextObject, FExFlowStateData StateData, EExReplicationMode Mode)
{
	auto* Proxy = NewObject<UExReplicationProxy>();
	Proxy->FlowState = StateData;
	Proxy->ReplicationMode = Mode;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExReplicationProxy* UExReplicationProxy::CreateProxy_ServerValidate(UObject* WorldContextObject, FExFlowStateData StateData)
{
	auto* Proxy = NewObject<UExReplicationProxy>();
	Proxy->FlowState = StateData;
	Proxy->ReplicationMode = EExReplicationMode::ServerOnly;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExReplicationProxy::UpdateFlowState(FExFlowStateData NewState)
{
	FlowState = NewState;
}

void UExSaveGameProxy::Activate()
{
	Super::Activate();
	SetReadyToDestroy();
}

UExSaveGameProxy* UExSaveGameProxy::CreateProxy_SaveGame(UObject* WorldContextObject, FString InSlotName, int32 InUserIndex, FExFlowStateData FlowState)
{
	auto* Proxy = NewObject<UExSaveGameProxy>();
	Proxy->SlotName = InSlotName;
	Proxy->UserIndex = InUserIndex;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExSaveGameProxy* UExSaveGameProxy::CreateProxy_LoadGame(UObject* WorldContextObject, FString InSlotName, int32 InUserIndex)
{
	auto* Proxy = NewObject<UExSaveGameProxy>();
	Proxy->SlotName = InSlotName;
	Proxy->UserIndex = InUserIndex;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExCheckpointProxy::Activate()
{
	Super::Activate();
	CreateCheckpointTimer();
}

UExCheckpointProxy* UExCheckpointProxy::CreateProxy_CreateCheckpoint(UObject* WorldContextObject, FExFlowStateData FlowState, float Interval)
{
	auto* Proxy = NewObject<UExCheckpointProxy>();
	Proxy->CheckpointData = FlowState;
	Proxy->CheckpointInterval = Interval;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExCheckpointProxy::UpdateCheckpoint(FExFlowStateData NewState)
{
	CheckpointData = NewState;
}

FExFlowStateData UExCheckpointProxy::GetLastCheckpoint() const
{
	return CheckpointData;
}

void UExCheckpointProxy::RestoreFromCheckpoint()
{
	OnCheckpointCreated.Broadcast(true);
}

void UExCheckpointProxy::CreateCheckpointTimer()
{
	UWorld* World = GetWorld();
	if (!World || CheckpointInterval <= 0.f)
	{
		return;
	}

	FTimerManager& TimerManager = World->GetTimerManager();
	TimerManager.SetTimer(CheckpointTimerHandle, [this]()
	{
		OnCheckpointCreated.Broadcast(true);
	}, CheckpointInterval, true);
}
