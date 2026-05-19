// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlueprintTool/Subsystems/ExLatentActionManager.h"
#include "ExProxy_BlendPercent.generated.h"


/**
 * Wait Multi-Input Proxy Result
 * Check Condition
 * Execute Once
 */
UCLASS()
class BLUEPRINTNODEGRAPH_API UExProxy_BlendPercent : public UExBase_FlowProxy, public FTickableGameObject
{
	GENERATED_BODY()

	friend UExBase_FlowProxy;
	
	bool bInputOk=false;
	bool bEnableTick=false;
	
	// condition value
	float* InputPercentSpeed1=nullptr;
	float* InputPercentSpeed2=nullptr;

	float FinalPercent=0.f;

public:
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UExProxy_BlendPercent, STATGROUP_Tickables);
	}
	virtual bool IsTickable() const override {return bEnableTick;}
	virtual void Tick(float DeltaTime) override;
	
public:
	virtual ~UExProxy_BlendPercent() override
	{
		FinalPercent = 0.f;
		InputPercentSpeed1 = nullptr;
		InputPercentSpeed2 = nullptr;
	}
	/**
	 * CreateProxy Function
	 * Function params should Equal to PinNames
	 * InputCountPinName
	 * UUIDPinName
	 */
	UFUNCTION(BlueprintCallable, Category="Utilities|FlowControl", meta=(WorldContext="WorldContextObject",HidePin="UUID,InputCount"))
	static UExProxy_BlendPercent*	CreateProxy(UObject* WorldContextObject, FString UUID, int32 InputCount, UPARAM(ref) float& PercentSpeed1, UPARAM(ref) float& PercentSpeed2);

protected:
	// wait branches
	virtual void OnBranchesFinished() override;
	
	virtual void OnFinishCall() override;
};
