// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExAsyncActionBase.h"
#include "ExLatentActionManager.h"
#include "ExWaitBranchProxy.generated.h"

/**
 * Wait Multi-Input Proxy
 * Delay To Execute Once
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExWaitBranchProxy : public UExLatentActionProxyBase
{
	GENERATED_BODY()

public:
	/**
	 * CreateProxy Function - (UExK2Node_WaitInput - create pins)
	 * Function params should Equal to PinNames
	 * InputCountPinName
	 * UUIDPinName
	 */
	UFUNCTION(BlueprintCallable, Category="Utilities|FlowControl", meta=(WorldContext="WorldContextObject",HidePin="UUID,InputCount"))
	static UExWaitBranchProxy* CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount);

protected:
	virtual bool IsFinishAfterBranches() const override { return true; }
	// wait branches
	virtual void OnBranchesFinished() override;
};
