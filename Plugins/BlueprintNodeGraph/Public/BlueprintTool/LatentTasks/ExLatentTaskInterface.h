// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Async/IAsyncTask.h"
#include "BlueprintTool/Common/ExLatentProxyDefine.h"
#include "ExLatentTaskInterface.generated.h"

/** Latent task lifecycle state */
UENUM(BlueprintType)
enum class EExLatentTaskState : uint8
{
	/** Task was cancelled */
	Cancelled,

	/** Task finished successfully */
	Completed,

	/** Task failed */
	Failed,

	/** Task is waiting to start */
	Pending,

	/** Task is running */
	Running,
};

/**
 * @class UExLatentTaskInterface
 * @brief UInterface wrapper for latent task API (not implementable in Blueprint).
 */
UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UExLatentTaskInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @class IExLatentTaskInterface
 * @brief Core latent task interface: state queries and lifecycle control.
 */
class BLUEPRINTNODEGRAPH_API IExLatentTaskInterface
{
	GENERATED_BODY()

public:
	// ========== Context ==========

	/** Whether this task runs on the local autonomous client */
	virtual bool IsLocal() { return false; }

	// ========== State Management ==========

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual EExLatentTaskState GetState() const = 0;

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual void SetState(EExLatentTaskState InState) = 0;

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual FString GetStateName() const;

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual bool IsPending() const { return GetState() == EExLatentTaskState::Pending; }

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual bool IsRunning() const { return GetState() == EExLatentTaskState::Running; }

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual bool IsStopped() const { return GetState() == EExLatentTaskState::Completed; }

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual bool IsCancelled() const { return GetState() == EExLatentTaskState::Cancelled; }

	// ========== Task Control ==========

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual void TryReset();

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual void TryStart();

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual void TryStop();

	UFUNCTION(BlueprintCallable, Category = "LatentTask")
	virtual void Terminate();

protected:
	UFUNCTION()
	virtual void PreOnStart() {}

	UFUNCTION()
	virtual void OnStart() {}

	/** Called after the full OnStart chain completes; use for blueprint notification. */
	UFUNCTION()
	virtual void PostOnStart() {}

	UFUNCTION()
	virtual void OnStop() {}
};
