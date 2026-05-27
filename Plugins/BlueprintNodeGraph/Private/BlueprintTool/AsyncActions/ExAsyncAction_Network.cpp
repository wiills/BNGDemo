// Copyright BlueprintNodeGraph. All Rights Reserved.

#include "BlueprintTool/AsyncActions/ExAsyncAction_Network.h"
#include "Engine/World.h"

void UExAsyncAction_Replication::Activate()
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
		UE_LOG(LogAsyncAction, Display, TEXT("[UExAsyncAction_Replication] Multicast replication for state: %s"), *FlowState.StateName);
	}
	else if (ReplicationMode == EExReplicationMode::ServerOnly)
	{
		if (World->GetNetMode() == NM_Client)
		{
			SetReadyToDestroy();
			return;
		}
		OnReplicated.Broadcast();
		UE_LOG(LogAsyncAction, Display, TEXT("[UExAsyncAction_Replication] Server-only replication for state: %s"), *FlowState.StateName);
	}

	SetReadyToDestroy();
}

UExAsyncAction_Replication* UExAsyncAction_Replication::CreateProxy_Replicate(UObject* WorldContextObject, FExFlowStateData StateData, EExReplicationMode Mode)
{
	auto* Proxy = NewObject<UExAsyncAction_Replication>();
	Proxy->FlowState = StateData;
	Proxy->ReplicationMode = Mode;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExAsyncAction_Replication* UExAsyncAction_Replication::CreateProxy_ServerValidate(UObject* WorldContextObject, FExFlowStateData StateData)
{
	auto* Proxy = NewObject<UExAsyncAction_Replication>();
	Proxy->FlowState = StateData;
	Proxy->ReplicationMode = EExReplicationMode::ServerOnly;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExAsyncAction_Replication::UpdateFlowState(FExFlowStateData NewState)
{
	FlowState = NewState;
}

void UExAsyncAction_SaveGame::Activate()
{
	Super::Activate();
	SetReadyToDestroy();
}

UExAsyncAction_SaveGame* UExAsyncAction_SaveGame::CreateProxy_SaveGame(UObject* WorldContextObject, FString InSlotName, int32 InUserIndex, FExFlowStateData FlowState)
{
	auto* Proxy = NewObject<UExAsyncAction_SaveGame>();
	Proxy->SlotName = InSlotName;
	Proxy->UserIndex = InUserIndex;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

UExAsyncAction_SaveGame* UExAsyncAction_SaveGame::CreateProxy_LoadGame(UObject* WorldContextObject, FString InSlotName, int32 InUserIndex)
{
	auto* Proxy = NewObject<UExAsyncAction_SaveGame>();
	Proxy->SlotName = InSlotName;
	Proxy->UserIndex = InUserIndex;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExAsyncAction_Checkpoint::Activate()
{
	Super::Activate();
	CreateCheckpointTimer();
}

UExAsyncAction_Checkpoint* UExAsyncAction_Checkpoint::CreateProxy_CreateCheckpoint(UObject* WorldContextObject, FExFlowStateData FlowState, float Interval)
{
	auto* Proxy = NewObject<UExAsyncAction_Checkpoint>();
	Proxy->CheckpointData = FlowState;
	Proxy->CheckpointInterval = Interval;
	Proxy->RegisterWithGameInstance(WorldContextObject);
	return Proxy;
}

void UExAsyncAction_Checkpoint::UpdateCheckpoint(FExFlowStateData NewState)
{
	CheckpointData = NewState;
}

FExFlowStateData UExAsyncAction_Checkpoint::GetLastCheckpoint() const
{
	return CheckpointData;
}

void UExAsyncAction_Checkpoint::RestoreFromCheckpoint()
{
	OnCheckpointCreated.Broadcast(true);
}

void UExAsyncAction_Checkpoint::CreateCheckpointTimer()
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
