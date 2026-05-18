// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ExAsyncActionBase.h"
#include "ExLatentActionManager.h"
#include "ExWaitConditionProxy.generated.h"


/**
 * Wait Multi-Input Proxy Result
 * Check Condition
 * Execute Once
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExWaitConditionProxy : public UExLatentActionProxyBase, public FTickableGameObject
{
	GENERATED_BODY()

	friend UExLatentActionProxyBase;
	
	bool bInputOk=false;
	bool bEnableTick=false;
	
	// condition value Pointer
	bool* bSelfConditionValuePtr=nullptr;

protected:
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UExWaitConditionProxy, STATGROUP_Tickables);
	}
	virtual bool IsTickable() const override {return bEnableTick;}
	virtual void Tick(float DeltaTime) override;
	
public:
	virtual ~UExWaitConditionProxy() override
	{
		bSelfConditionValuePtr = nullptr;
	}
	/**
	 * CreateProxy Function
	 * Function params should Equal to PinNames
	 * InputCountPinName
	 * UUIDPinName
	 */
	UFUNCTION(BlueprintCallable, Category="Utilities|FlowControl", meta=(WorldContext="WorldContextObject",HidePin="UUID,InputCount"))
	static UExWaitConditionProxy* CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount, UPARAM(ref) bool& Condition);

protected:
	// wait branches
	virtual void OnBranchesFinished() override;
	
	virtual void OnFinishCall() override;
};
