// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/LatentTasks/ExBase_LatentTask.h"
#include "ExLatentTask_Custom.generated.h"

/**
 * @class UExLatentTask_Custom
 * @brief User-defined latent task (blueprint subclass base).
 *
 * Created via ExK2Node_LatentTaskObject / CreateProxy.
 * For quest latent tasks inherit UExLatentTask_Quest and use UExK2Node_QuestTask (not CreateLatentTask).
 */
UCLASS(Blueprintable, BlueprintType, HideDropdown, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen))
class BLUEPRINTNODEGRAPH_API UExLatentTask_Custom : public UExBase_LatentTask
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "LatentTasks", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", DisplayName = "CreateLatentTask"))
	static UExLatentTask_Custom* CreateProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_Custom> Class);
};

/**
 * @class UExLatentTask_BranchSync
 * @brief Multi-branch sync latent task (internal K2 use).
 */
UCLASS(NotBlueprintable, NotBlueprintType, HideDropdown, meta = (ExposedAsyncProxy = AsyncTask, SafeHideThen))
class BLUEPRINTNODEGRAPH_API UExLatentTask_BranchSync : public UExBase_LatentTask
{
	GENERATED_BODY()

	UPROPERTY()
	bool bBranchesFinished = false;

	UPROPERTY()
	FString m_SelfUUID;

	UPROPERTY()
	int32 m_InputBranchCount;

public:
	void SetUUIDAndCount(FString UUID, int32 InCount)
	{
		m_SelfUUID = UUID;
		m_InputBranchCount = InCount;
	}

	virtual bool IsBranchesFinished() const { return bBranchesFinished; }

	UFUNCTION(BlueprintCallable, Category = "ExTasks", meta = (BlueprintInternalUseOnly = "true", DisplayName = "CreateLatentTaskBranchSync", WorldContext = "WorldContextObject", HidePin = "UUID,InputCount"))
	static UExLatentTask_BranchSync* CreateProxy(UObject* WorldContextObject, TSubclassOf<UExLatentTask_BranchSync> Class, FString UUID, int32 InputCount);

	virtual void Activate() override final
	{
		UE_LOG(LogLatentTask, Display, TEXT("[UExLatentTask_BranchSync::Activate] - %s, Count: %d"), *GetName(), m_InputBranchCount);
		m_InputBranchCount--;
		if (m_InputBranchCount <= 0 && !IsStopped())
		{
			bBranchesFinished = true;
			OnBranchesFinished();
			TryStop();
		}
	}

protected:
	virtual void OnBranchesFinished() {}
	virtual void OnStop() override;
	void RemoveWaitInstance() const;
};
