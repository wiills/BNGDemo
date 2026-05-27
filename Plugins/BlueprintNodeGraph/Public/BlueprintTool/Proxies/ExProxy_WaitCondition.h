// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/Subsystems/ExLatentActionManager.h"
#include "ExProxy_WaitCondition.generated.h"


/**
 * Wait Multi-Input Proxy Result
 * Check Condition
 * Execute Once
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExProxy_WaitCondition : public UExBase_FlowProxy, public FTickableGameObject
{
	GENERATED_BODY()

	friend UExBase_FlowProxy;
	
	bool bInputOk=false;
	bool bEnableTick=false;
	
	// condition value Pointer
	bool* bSelfConditionValuePtr=nullptr;

public:
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UExProxy_WaitCondition, STATGROUP_Tickables);
	}
	virtual bool IsTickable() const override {return bEnableTick;}
	virtual void Tick(float DeltaTime) override;
	
public:
	virtual ~UExProxy_WaitCondition() override
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
	static UExProxy_WaitCondition* CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount, UPARAM(ref) bool& Condition);

protected:
	// wait branches
	virtual void OnBranchesFinished() override;
	virtual bool IsCustomFinish() const override { return true; }
	
	virtual void OnFinishCall() override;
};
