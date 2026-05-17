// Copyright BlueprintNodeGraph. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ExAsyncActionBase.h"
#include "ExNetworkProxy.generated.h"

UENUM(BlueprintType)
enum class EExReplicationMode : uint8
{
	None,
	ServerOnly,
	Multicast
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReplicationSyncDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveGameDelegate, bool, bSuccess);

USTRUCT(BlueprintType)
struct FExFlowStateData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	FString StateName;

	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	TMap<FString, FString> Variables;

	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	int32 CurrentNodeId = 0;

	UPROPERTY(BlueprintReadWrite, Category = "FlowState")
	float ElapsedTime = 0.f;
};

UCLASS()
class BLUEPRINTNODEGRAPH_API UExReplicationProxy : public UExAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Network")
	FOnReplicationSyncDelegate OnReplicated;

protected:
	UPROPERTY()
	EExReplicationMode ReplicationMode = EExReplicationMode::None;

	UPROPERTY()
	FExFlowStateData FlowState;

public:
	UFUNCTION(BlueprintCallable, Category = "Network", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Replicate Flow State"))
	static UExReplicationProxy* CreateProxy_Replicate(UObject* WorldContextObject, FExFlowStateData StateData, EExReplicationMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Network", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Server Validate Flow"))
	static UExReplicationProxy* CreateProxy_ServerValidate(UObject* WorldContextObject, FExFlowStateData StateData);

	UFUNCTION(BlueprintCallable, Category = "Network", meta = (BlueprintInternalUseOnly = "true"))
	void UpdateFlowState(FExFlowStateData NewState);

protected:
	virtual void Activate() override;
};

UCLASS()
class BLUEPRINTNODEGRAPH_API UExSaveGameProxy : public UExAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "SaveGame")
	FOnSaveGameDelegate OnSaveComplete;

	UPROPERTY(BlueprintAssignable, Category = "SaveGame")
	FOnSaveGameDelegate OnLoadComplete;

protected:
	UPROPERTY()
	FString SlotName;

	UPROPERTY()
	int32 UserIndex = 0;

public:
	UFUNCTION(BlueprintCallable, Category = "SaveGame", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Save Flow State"))
	static UExSaveGameProxy* CreateProxy_SaveGame(UObject* WorldContextObject, FString InSlotName, int32 InUserIndex, FExFlowStateData FlowState);

	UFUNCTION(BlueprintCallable, Category = "SaveGame", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Load Flow State"))
	static UExSaveGameProxy* CreateProxy_LoadGame(UObject* WorldContextObject, FString InSlotName, int32 InUserIndex);

protected:
	virtual void Activate() override;
};

UCLASS()
class BLUEPRINTNODEGRAPH_API UExCheckpointProxy : public UExAsyncActionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Checkpoint")
	FOnSaveGameDelegate OnCheckpointCreated;

protected:
	UPROPERTY()
	FExFlowStateData CheckpointData;

	UPROPERTY()
	float CheckpointInterval = 30.f;

	UPROPERTY()
	FTimerHandle CheckpointTimerHandle;

public:
	UFUNCTION(BlueprintCallable, Category = "Checkpoint", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "Create Checkpoint"))
	static UExCheckpointProxy* CreateProxy_CreateCheckpoint(UObject* WorldContextObject, FExFlowStateData FlowState, float Interval);

	UFUNCTION(BlueprintCallable, Category = "Checkpoint", meta = (BlueprintInternalUseOnly = "true"))
	void UpdateCheckpoint(FExFlowStateData NewState);

	UFUNCTION(BlueprintCallable, Category = "Checkpoint", meta = (BlueprintInternalUseOnly = "true"))
	FExFlowStateData GetLastCheckpoint() const;

	UFUNCTION(BlueprintCallable, Category = "Checkpoint", meta = (BlueprintInternalUseOnly = "true"))
	void RestoreFromCheckpoint();

protected:
	virtual void Activate() override;
	void CreateCheckpointTimer();
};
